#include "rt_loop.h"
#include "rt_http.h"
#include "rt_ws.h"
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <vector>
#include <unordered_map>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

namespace {
struct Conn {
    std::string rbuf;
    std::deque<std::string> wq;
    size_t woff = 0;
    size_t wqBytes = 0; // queued unsent bytes (spec cap 256KB, drop not OOM)
    bool closed = false;
    bool isWs = false;
    bool sse = false; // SSE stream: held open, inbound bytes discarded
    std::string frag;      // fragmented message reassembly
    uint8_t fragOp = 0x1;
    int64_t lastActiveMs = 0;
    bool pingOut = false;
    // HTTP body accumulation (Content-Length only; chunked deferred)
    bool wantBody = false;
    size_t bodyNeed = 0;
    RtLoop::HttpRequestSnap pending;
    bool wantStats = false; // GET /__stats (served inline, no worker)
};
enum class WKind { Http, Connect, Message, Close };
struct Work { WKind kind; RtLoop::HttpRequestSnap snap; int fd; std::string payload; uint8_t opcode = 0x1; uint64_t seq = 0; };
struct Resp { int fd; std::string data; };

std::mutex wmtx, rmtx;
std::condition_variable wcv;
std::queue<Work> wq;
std::queue<Resp> rq;
std::unordered_map<int, Conn> conns;
std::mutex cmtx;
std::vector<std::thread> workers;
std::thread loopTh;
std::atomic<bool> stopFlag{false};
std::atomic<uint64_t> seqGen{0};
int wakeR_ = -1, wakeW_ = -1; // self-pipe: workers wake loop for queued sends

int64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
void setNonblock(int fd) {
    int f = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, f | O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
}
void tuneClient(int fd) {
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#ifdef TCP_QUICKACK
    setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &one, sizeof(one));
#endif
    int sz = 16 * 1024;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz));
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sz, sizeof(sz));
}
inline void quickAck(int fd) {
#ifdef TCP_QUICKACK
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &one, sizeof(one));
#endif
}
std::string httpResp(const std::string& body, bool keepAlive) {
    std::string h = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
        + std::to_string(body.size()) + "\r\nConnection: "
        + (keepAlive ? "keep-alive" : "close") + "\r\n\r\n";
    return h + body;
}
void workerFn() {
    RtLoop& loop = RtLoop::instance();
    auto timed = [&](auto&& fn) {
        auto t0 = std::chrono::steady_clock::now();
        fn();
        auto t1 = std::chrono::steady_clock::now();
        loop.noteLatencyMs((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    };
    while (!stopFlag) {
        Work w;
        { std::unique_lock<std::mutex> l(wmtx);
          wcv.wait_for(l, std::chrono::milliseconds(100), []{ return !wq.empty() || stopFlag.load(); });
          if (stopFlag) return;
          if (wq.empty()) continue;
          w = std::move(wq.front()); wq.pop(); }
        try {
            if (w.kind == WKind::Http) {
                RtLoop::HttpResponse resp;
                timed([&]{ auto cb = loop.onRequest;
                    if (cb) resp = cb(w.snap, w.fd);
                    else resp.body = "LAML v4.1 rt: no on(\"request\") handler. Use on(\"request\", func(req){...}).\n"; });
                if (!resp.sse) {
                    std::lock_guard<std::mutex> l(rmtx);
                    rq.push({w.fd, httpResp(resp.body, true)});
                }
                // sse: bridge already sent headers via sendRaw + markSse
            } else if (w.kind == WKind::Connect) {
                timed([&]{ auto cb = loop.onConnect;
                    if (cb) cb(w.fd, w.payload); });
            } else if (w.kind == WKind::Message) {
                timed([&]{ auto cb = loop.onMessage;
                    if (cb) cb(w.fd, std::move(w.payload), w.opcode); });
            } else if (w.kind == WKind::Close) {
                auto cb = loop.onClose;
                if (cb) cb(w.fd);
            }
        } catch (...) {}
    }
}
void pushWork(Work&& w) {
    { std::lock_guard<std::mutex> l(wmtx); wq.push(std::move(w)); }
    wcv.notify_one();
}
// Queue bytes to a conn with the 256KB spec cap. Caller must hold cmtx.
// Returns false when over cap (caller should close + count drop).
inline bool qpush(Conn& c, std::string s, std::atomic<uint64_t>& dropped) {
    if (c.wqBytes + s.size() > 256 * 1024) { dropped++; return false; }
    c.wqBytes += s.size();
    c.wq.push_back(std::move(s));
    return true;
}
}

bool RtLoop::start(int port, int nworkers) {
    if (running_) return true;
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) return false;
    int one = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#ifdef SO_REUSEPORT
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif
    fcntl(listenFd_, F_SETFD, FD_CLOEXEC);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons((uint16_t)port);
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenFd_, (sockaddr*)&a, sizeof(a)) < 0) { close(listenFd_); listenFd_=-1; return false; }
    if (listen(listenFd_, 4096) < 0) { close(listenFd_); listenFd_=-1; return false; }
    setNonblock(listenFd_);
    sockaddr_in got{}; socklen_t gl = sizeof(got);
    if (getsockname(listenFd_, (sockaddr*)&got, &gl) == 0) boundPort_ = ntohs(got.sin_port);
    else boundPort_ = port;
    if (nworkers <= 0) nworkers = 4;
    if (nworkers > 32) nworkers = 32;
    int wake[2];
    if (pipe(wake) == 0) {
        wakeR_ = wake[0]; wakeW_ = wake[1];
        setNonblock(wakeR_); setNonblock(wakeW_);
    }
    stopFlag = false;
    running_ = true;
    startMs_ = nowMs();
    for (int i = 0; i < nworkers; i++) workers.emplace_back(workerFn);
    loopTh = std::thread([this]{
        int64_t lastSweep = nowMs();
        while (!stopFlag) {
            std::vector<pollfd> pfds;
            pfds.reserve(256);
            pollfd lp{}; lp.fd = listenFd_; lp.events = POLLIN; pfds.push_back(lp);
            if (wakeR_ >= 0) { pollfd wp{}; wp.fd = wakeR_; wp.events = POLLIN; pfds.push_back(wp); }
            std::vector<int> fds;
            { std::lock_guard<std::mutex> l(cmtx);
              for (auto& kv : conns) { if (!kv.second.closed) fds.push_back(kv.first); } }
            for (int fd : fds) { pollfd p{}; p.fd = fd; p.events = POLLIN | POLLOUT; pfds.push_back(p); }
            int n = poll(pfds.data(), pfds.size(), 50);
            if (stopFlag) break;
            size_t cli0 = 1; // first client slot in pfds
            if (wakeR_ >= 0) {
                cli0 = 2;
                if (pfds[1].revents & POLLIN) {
                    char tmp[256];
                    while (recv(wakeR_, tmp, sizeof(tmp), 0) > 0) {}
                }
            }
            if (n > 0) {
                if (pfds[0].revents & POLLIN) {
                    while (true) {
                        int c = accept(listenFd_, nullptr, nullptr);
                        if (c < 0) break;
                        setNonblock(c); tuneClient(c);
                        std::lock_guard<std::mutex> l(cmtx);
                        Conn cc; cc.lastActiveMs = nowMs();
                        conns[c] = std::move(cc);
                        accepts_++;
                    }
                }
                for (size_t i = cli0; i < pfds.size(); i++) {
                    int fd = pfds[i].fd;
                    short re = pfds[i].revents;
                    if (re & (POLLERR | POLLNVAL)) {
                        std::lock_guard<std::mutex> l(cmtx);
                        auto it = conns.find(fd);
                        if (it != conns.end()) { close(fd); conns.erase(it); pushWork({WKind::Close, {}, fd, "", 0, ++seqGen}); }
                        continue;
                    }
                    if (re & (POLLHUP) && !(re & POLLIN)) {
                        std::lock_guard<std::mutex> l(cmtx);
                        auto it = conns.find(fd);
                        if (it != conns.end()) { close(fd); conns.erase(it); pushWork({WKind::Close, {}, fd, "", 0, ++seqGen}); }
                        continue;
                    }
                    if (re & POLLIN) {
                        char buf[8192];
                        bool dead = false;
                        while (!dead) {
                            ssize_t r = recv(fd, buf, sizeof(buf), 0);
                            if (r > 0) {
                                quickAck(fd);
                                std::lock_guard<std::mutex> l(cmtx);
                                auto it = conns.find(fd);
                                if (it == conns.end()) break;
                                Conn& c = it->second;
                                c.lastActiveMs = nowMs(); c.pingOut = false;
                                if (c.sse) {
                                    // Held-open SSE stream: discard inbound, stay alive.
                                    // (r==0 close handled below; data just ignored.)
                                    if (r < (ssize_t)sizeof(buf)) break;
                                    continue;
                                }
                                if (!c.isWs) {
                                    c.rbuf.append(buf, (size_t)r);
                                    while (true) {
                                        // finish a pending body first
                                        if (c.wantBody) {
                                            if (c.rbuf.size() < c.bodyNeed) break;
                                            c.pending.body = c.rbuf.substr(0, c.bodyNeed);
                                            c.rbuf.erase(0, c.bodyNeed);
                                            c.wantBody = false;
                                            // wantStats stays set: serving pass below answers it
                                            if (!c.wantStats) { pushWork({WKind::Http, c.pending, fd, "", 0, ++seqGen}); reqs_++; }
                                            continue;
                                        }
                                        HttpRequest req;
                                        long hl = HttpParser::parseHead(c.rbuf.data(), c.rbuf.size(), req);
                                        if (hl < 0) { close(fd); conns.erase(it); pushWork({WKind::Close, {}, fd, "", 0, ++seqGen}); dead = true; break; }
                                        if (hl == 0) break;
                                        c.rbuf.erase(0, (size_t)hl);
                                        // WS upgrade?
                                        bool wantWs = req.isUpgradeWs && hasWsHandler && hasWsHandler();
                                        if (wantWs) {
                                            std::string acc = WsCodec::acceptKey(req.wsKey);
                                            std::string resp = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + acc + "\r\n\r\n";
                                            if (!qpush(c, std::move(resp), dropped_)) { c.closed = true; break; }
                                            c.isWs = true;
                                            c.rbuf.clear();
                                            HttpRequestSnap s{req.method, req.path, req.query, ""};
                                            pushWork({WKind::Connect, s, fd, req.path, 0, ++seqGen});
                                        } else {
                                            // Content-Length body? (cap 1MB; chunked -> close in 4.1)
                                            size_t clen = 0;
                                            auto hcl = req.headers.find("content-length");
                                            if (hcl != req.headers.end()) {
                                                try { clen = (size_t)stoul(hcl->second); } catch (...) { clen = 0; }
                                                if (clen > 1024*1024) { close(fd); conns.erase(it); pushWork({WKind::Close, {}, fd, "", 0, ++seqGen}); dead = true; break; }
                                            }
                                            HttpRequestSnap s{req.method, req.path, req.query, ""};
                                            bool isStats = (statsEndpoint_ && req.path == "/__stats");
                                            if (clen > 0) {
                                                c.pending = s;
                                                c.bodyNeed = clen;
                                                c.wantBody = true;
                                                c.wantStats = isStats;
                                                continue; // loop back to body branch
                                            }
                                            if (isStats) { c.wantStats = true; }
                                            else { pushWork({WKind::Http, s, fd, "", 0, ++seqGen}); reqs_++; }
                                        }
                                        if (c.rbuf.empty() && !c.wantBody) break;
                                    }
                                } else {
                                    // WS frames
                                    c.rbuf.append(buf, (size_t)r);
                                    while (true) {
                                        size_t used = 0; bool bad = false;
                                        auto fr = WsCodec::decode(c.rbuf.data(), c.rbuf.size(), used, bad);
                                        if (bad) {
                                            if (!qpush(c, WsCodec::encodeClose(1002, "protocol error"), dropped_)) { /* already full */ }
                                            c.closed = true;
                                            break;
                                        }
                                        if (!fr) break;
                                        c.rbuf.erase(0, used);
                                        if (fr->opcode == 0x9) { // ping -> pong
                                            qpush(c, WsCodec::encode(fr->payload, 0xA), dropped_);
                                        } else if (fr->opcode == 0xA) {
                                            // pong: alive
                                        } else if (fr->opcode == 0x8) {
                                            qpush(c, WsCodec::encodeClose(1000), dropped_);
                                            c.closed = true;
                                            pushWork({WKind::Close, {}, fd, "", 0, ++seqGen});
                                            break;
                                        } else if (fr->opcode == 0x0) { // continuation
                                            c.frag += fr->payload;
                                            if (c.frag.size() > WsCodec::kMaxFrame) { qpush(c, WsCodec::encodeClose(1009, "too big"), dropped_); c.closed = true; break; }
                                            if (fr->fin) {
                                                std::string msg = std::move(c.frag); c.frag.clear();
                                                uint8_t op = c.fragOp;
                                                wsMsgs_++;
                                                pushWork({WKind::Message, {}, fd, std::move(msg), op, ++seqGen});
                                            }
                                        } else { // text/binary
                                            if (!fr->fin) { c.frag = std::move(fr->payload); c.fragOp = fr->opcode; }
                                            else { wsMsgs_++; pushWork({WKind::Message, {}, fd, std::move(fr->payload), fr->opcode, ++seqGen}); }
                                        }
                                    }
                                }
                                if (r < (ssize_t)sizeof(buf)) break;
                            } else if (r == 0) {
                                std::lock_guard<std::mutex> l(cmtx);
                                auto it = conns.find(fd);
                                if (it != conns.end()) { close(fd); conns.erase(it); pushWork({WKind::Close, {}, fd, "", 0, ++seqGen}); }
                                break;
                            } else {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                                if (errno == EINTR) continue;
                                break;
                            }
                        }
                        (void)dead;
                    }
                }
            }
            // heartbeat sweep every 10s: ping idle WS, kill dead
            int64_t now = nowMs();
            if (now - lastSweep > 10000) {
                lastSweep = now;
                std::lock_guard<std::mutex> l(cmtx);
                for (auto& kv : conns) {
                    Conn& c = kv.second;
                    if (!c.isWs || c.closed) continue;
                    int64_t idle = now - c.lastActiveMs;
                    if (idle > 114000) c.closed = true;
                    else if (idle > 54000 && !c.pingOut) {
                        if (qpush(c, WsCodec::encode("", 0x9), dropped_)) c.pingOut = true;
                    }
                }
            }
            // /__stats: serve inline (no worker), then clear flags
            { std::vector<int> sfds;
              { std::lock_guard<std::mutex> l(cmtx);
                for (auto& kv : conns) if (kv.second.wantStats) { kv.second.wantStats = false; sfds.push_back(kv.first); } }
              if (!sfds.empty()) {
                  Stats st = stats();
                  std::string body =
                      "# HELP laml_conns Active connections\nlaml_conns " + std::to_string(st.active) + "\n"
                      "# HELP laml_accepts_total Total accepted\nlaml_accepts_total " + std::to_string(st.accepts) + "\n"
                      "# HELP laml_requests_total HTTP requests\nlaml_requests_total " + std::to_string(st.reqs) + "\n"
                      "# HELP laml_ws_messages_total WS messages\nlaml_ws_messages_total " + std::to_string(st.wsMsgs) + "\n"
                      "# HELP laml_dropped_total Dropped sends\nlaml_dropped_total " + std::to_string(st.dropped) + "\n"
                      "# HELP laml_rps_avg Average events/sec since start\nlaml_rps_avg " + std::to_string(st.rpsAvg) + "\n"
                      "# HELP laml_handler_p50_ms Handler p50\nlaml_handler_p50_ms " + std::to_string(st.p50Ms) + "\n"
                      "# HELP laml_handler_p99_ms Handler p99\nlaml_handler_p99_ms " + std::to_string(st.p99Ms) + "\n";
                  std::string full = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
                      + std::to_string(body.size()) + "\r\nConnection: keep-alive\r\n\r\n" + body;
                  for (int fd : sfds) sendRaw(fd, full);
              } }
            // flush responses + queued WS sends
            while (true) {
                Resp r; bool has = false;
                { std::lock_guard<std::mutex> l(rmtx); if (!rq.empty()) { r = std::move(rq.front()); rq.pop(); has = true; } }
                if (!has) break;
                std::lock_guard<std::mutex> l(cmtx);
                auto it = conns.find(r.fd);
                if (it == conns.end()) { dropped_++; continue; }
                // Spec backpressure: 256KB per-fd cap, drop (counted) not OOM.
                if (it->second.wqBytes + r.data.size() > 256 * 1024) { dropped_++; continue; }
                it->second.wqBytes += r.data.size();
                it->second.wq.push_back(std::move(r.data));
            }
                { std::lock_guard<std::mutex> l(cmtx);
                  for (auto& kv : conns) {
                      Conn& c = kv.second;
                      while (!c.wq.empty()) {
                          std::string& front = c.wq.front();
                          const char* p = front.data() + c.woff;
                          size_t left = front.size() - c.woff;
                          ssize_t w = ::send(kv.first, p, left, MSG_NOSIGNAL);
                          if (w > 0) { c.woff += (size_t)w; c.wqBytes -= (size_t)w; if (c.woff >= front.size()) { c.wq.pop_front(); c.woff = 0; } }
                          else { if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) break; else { c.closed = true; break; } }
                          if (w < (ssize_t)left) break;
                  }
              }
              for (auto it = conns.begin(); it != conns.end();) {
                  if (it->second.closed && it->second.wq.empty()) { close(it->first); int fd=it->first; it = conns.erase(it); pushWork({WKind::Close, {}, fd, "", 0, ++seqGen}); }
                  else if (it->second.closed) { ++it; } // drain queue first
                  else ++it;
              }
            }
        }
    });
    return true;
}

void RtLoop::stop() {
    if (!running_) return;
    stopFlag = true;
    wcv.notify_all();
    if (loopTh.joinable()) loopTh.join();
    for (auto& t : workers) if (t.joinable()) t.join();
    workers.clear();
    { std::lock_guard<std::mutex> l(cmtx);
      for (auto& kv : conns) close(kv.first);
      conns.clear(); }
    if (listenFd_ >= 0) { close(listenFd_); listenFd_ = -1; }
    if (wakeR_ >= 0) { close(wakeR_); wakeR_ = -1; }
    if (wakeW_ >= 0) { close(wakeW_); wakeW_ = -1; }
    running_ = false;
}

void RtLoop::sendRaw(int fd, std::string data) {
    if (data.size() > WsCodec::kMaxFrame + 16) return; // refuse absurd single send
    { std::lock_guard<std::mutex> l(rmtx);
      if (rq.size() > 65536) { dropped_++; return; }
      rq.push({fd, std::move(data)}); }
    if (wakeW_ >= 0) { char b = 0; (void)write(wakeW_, &b, 1); } // best-effort wake
}

void RtLoop::sendWs(int fd, const std::string& payload, uint8_t opcode) {
    sendRaw(fd, WsCodec::encode(payload, opcode));
}

void RtLoop::sendSmart(int fd, const std::string& payload) {
    std::string framed;
    { std::lock_guard<std::mutex> l(cmtx);
      auto it = conns.find(fd);
      if (it == conns.end()) return;
      if (it->second.isWs) framed = WsCodec::encode(payload);
      else if (it->second.sse) framed = "data: " + payload + "\n\n";
      else return; // plain HTTP fd: no open stream, drop
    }
    sendRaw(fd, std::move(framed));
}

void RtLoop::closeFd(int fd) {
    std::lock_guard<std::mutex> l(cmtx);
    auto it = conns.find(fd);
    if (it != conns.end()) it->second.closed = true;
}

void RtLoop::markSse(int fd) {
    std::lock_guard<std::mutex> l(cmtx);
    auto it = conns.find(fd);
    if (it != conns.end()) { it->second.sse = true; it->second.rbuf.clear(); }
}

RtLoop::Stats RtLoop::stats() {
    Stats s; s.accepts = accepts_; s.reqs = reqs_; s.wsMsgs = wsMsgs_; s.dropped = dropped_;
    { std::lock_guard<std::mutex> l(cmtx); s.active = conns.size(); }
    int64_t elapsed = nowMs() - startMs_;
    if (elapsed > 0) s.rpsAvg = (reqs_ + wsMsgs_) * 1000 / (uint64_t)elapsed;
    uint64_t total = latCount_.load();
    s.latCount = total;
    if (total > 0) {
        uint64_t acc = 0;
        for (int i = 0; i < 12; i++) {
            acc += latBuckets_[i].load();
            if (s.p50Ms == 0 && acc * 2 >= total) s.p50Ms = kBounds[i] == UINT64_MAX ? 5000 : kBounds[i];
            if (acc * 100 >= total * 99) {
                s.p99Ms = kBounds[i] == UINT64_MAX ? 5000 : kBounds[i];
                break;
            }
        }
    }
    return s;
}

void RtLoop::noteLatencyMs(uint64_t ms) {
    for (int i = 0; i < 12; i++) {
        if (ms <= kBounds[i]) { latBuckets_[i]++; break; }
    }
    latCount_++;
}

#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <array>
#include <cstdint>

// Bare-metal loop, language-agnostic. LAML never runs here.
// Single loop thread owns all fds; workers consume the work queue.
class RtLoop {
public:
    static RtLoop& instance() { static RtLoop l; return l; }

    struct HttpRequestSnap {
        std::string method, path, query, body;
    };
    struct HttpResponse {
        bool sse = false;      // hold open as SSE stream (headers sent by bridge)
        std::string body;      // normal body (wrapped in 200 + keep-alive)
    };

    bool start(int port, int workers = 4);
    void stop();
    bool running() const { return running_; }
    int port() const { return boundPort_; }

    // Thread-safe: queue bytes to fd (HTTP response bytes or WS frames).
    void sendRaw(int fd, std::string data);
    void sendStr(int fd, std::string data, bool closeAfter = false) { sendRaw(fd, std::move(data)); }
    void sendWs(int fd, const std::string& payload, uint8_t opcode = 0x1);
    // Transport-aware: WS frame for WS fds, SSE chunk for SSE fds, drop for plain HTTP.
    void sendSmart(int fd, const std::string& payload);
    void markSse(int fd); // hold HTTP fd open as SSE stream (skip further parsing)
    void closeFd(int fd);

    // Set by bridge. All run on WORKER threads, must be fast.
    std::function<HttpResponse(HttpRequestSnap, int)> onRequest;
    std::function<void(int, const std::string& path)> onConnect; // WS connect
    std::function<void(int, std::string, uint8_t)> onMessage;    // WS message (moved payload)
    std::function<void(int)> onClose;                            // any close
    std::function<bool()> hasWsHandler; // true if on(connect|message) registered
    std::atomic<bool> statsEndpoint_{false}; // serve({stats:true}) -> GET /__stats

    struct Stats {
        uint64_t accepts=0, active=0, reqs=0, wsMsgs=0, dropped=0;
        uint64_t rpsAvg=0, p50Ms=0, p99Ms=0, latCount=0;
    };
    Stats stats();
    // Called by workers around handler dispatch (lock-free).
    void noteLatencyMs(uint64_t ms);

private:
    RtLoop() = default;
    std::atomic<bool> running_{false};
    int listenFd_ = -1;
    int boundPort_ = 0;
    std::atomic<uint64_t> accepts_{0}, reqs_{0}, wsMsgs_{0}, dropped_{0};
    // Log-bucket latency histogram (upper bounds ms). Lock-free appends.
    static constexpr uint64_t kBounds[12] = {1,2,5,10,20,50,100,250,500,1000,5000,UINT64_MAX};
    std::array<std::atomic<uint64_t>,12> latBuckets_{};
    std::atomic<uint64_t> latCount_{0};
    int64_t startMs_ = 0;
};

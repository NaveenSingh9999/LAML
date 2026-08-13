#include "net_builtins.h"
#include "env.h"
#include "value.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

static Value err(const std::string& m) { return Value::makeError(m); }

// Numeric-address v1 (hostnames arrive with tls/PLAN P2 via getaddrinfo).
static bool parseAddr(const std::string& host, int port, sockaddr_in& out) {
    std::memset(&out, 0, sizeof(out));
    out.sin_family = AF_INET;
    out.sin_port = htons((uint16_t)port);
    if (host.empty()) {
        out.sin_addr.s_addr = htonl(INADDR_ANY);
        return true;
    }
    return inet_pton(AF_INET, host.c_str(), &out.sin_addr) == 1;
}

static Value builtinNetListen(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("netListen: expected port");
    int port = (int)args[0].intVal;
    std::string host;
    if (args.size() >= 2 && args[1].type == ValType::String) host = args[1].strVal;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return err("netListen: socket failed");
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in a;
    if (!parseAddr(host, port, a)) { close(fd); return err("netListen: bad address"); }
    if (bind(fd, (sockaddr*)&a, sizeof(a)) < 0) { close(fd); return err("netListen: bind failed"); }
    if (listen(fd, 128) < 0) { close(fd); return err("netListen: listen failed"); }
    return Value::makeInt(fd);
}

static Value builtinNetPort(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("netPort: expected fd");
    sockaddr_in a;
    socklen_t al = sizeof(a);
    if (getsockname((int)args[0].intVal, (sockaddr*)&a, &al) < 0)
        return err("netPort: getsockname failed");
    return Value::makeInt((int)ntohs(a.sin_port));
}

static Value builtinNetAccept(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("netAccept: expected fd");
    int fd = accept((int)args[0].intVal, nullptr, nullptr);
    if (fd < 0) return err("netAccept: accept failed");
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    return Value::makeInt(fd);
}

static Value builtinNetConnect(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String || args[1].type != ValType::Int)
        return err("netConnect: expected (host, port)");
    sockaddr_in a;
    if (!parseAddr(args[0].strVal, (int)args[1].intVal, a))
        return err("netConnect: bad address");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return err("netConnect: socket failed");
    if (connect(fd, (sockaddr*)&a, sizeof(a)) < 0) {
        close(fd);
        return err("netConnect: connection refused");
    }
    return Value::makeInt(fd);
}

static Value builtinNetRead(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::Int)
        return err("netRead: expected (fd, n)");
    int n = (int)args[1].intVal;
    if (n < 0) return err("netRead: n must be >= 0");
    std::string buf((size_t)n, '\0');
    ssize_t r;
    do {
        r = read((int)args[0].intVal, &buf[0], (size_t)n);
    } while (r < 0 && errno == EINTR);
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return err("netRead: timeout");
        return err("netRead: read failed");
    }
    buf.resize((size_t)r);
    return Value::makeString(buf);
}

static Value builtinNetWrite(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::String)
        return err("netWrite: expected (fd, data)");
    const std::string& d = args[1].strVal;
    size_t off = 0;
    while (off < d.size()) {
        ssize_t w;
        do {
            w = write((int)args[0].intVal, d.data() + off, d.size() - off);
        } while (w < 0 && errno == EINTR);
        if (w < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return err("netWrite: timeout");
            return err("netWrite: write failed");
        }
        off += (size_t)w;
    }
    return Value::makeInt((int64_t)off);
}

static Value builtinNetClose(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("netClose: expected fd");
    close((int)args[0].intVal);
    return NIL;
}

static Value builtinNetSetTimeout(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::Int)
        return err("netSetTimeout: expected (fd, ms)");
    int64_t ms = args[1].intVal;
    if (ms < 0) return err("netSetTimeout: ms must be >= 0");
    timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    int fd = (int)args[0].intVal;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    return NIL;
}

static Value builtinNetPeer(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("netPeer: expected fd");
    sockaddr_in a;
    socklen_t al = sizeof(a);
    if (getpeername((int)args[0].intVal, (sockaddr*)&a, &al) < 0)
        return err("netPeer: getpeername failed");
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &a.sin_addr, buf, sizeof(buf));
    return Value::makeString(std::string(buf) + ":" + std::to_string(ntohs(a.sin_port)));
}

static Value builtinUdpSocket(const std::vector<Value>&) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return err("udpSocket: socket failed");
    return Value::makeInt(fd);
}

static Value builtinUdpBind(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::Int)
        return err("udpBind: expected (fd, port)");
    sockaddr_in a;
    if (!parseAddr("", (int)args[1].intVal, a))
        return err("udpBind: bad address");
    if (bind((int)args[0].intVal, (sockaddr*)&a, sizeof(a)) < 0)
        return err("udpBind: bind failed");
    return NIL;
}

static Value builtinUdpSend(const std::vector<Value>& args) {
    if (args.size() < 4 || args[0].type != ValType::Int || args[1].type != ValType::String ||
        args[2].type != ValType::Int || args[3].type != ValType::String)
        return err("udpSend: expected (fd, host, port, data)");
    sockaddr_in a;
    if (!parseAddr(args[1].strVal, (int)args[2].intVal, a))
        return err("udpSend: bad address");
    ssize_t r = sendto((int)args[0].intVal, args[3].strVal.data(), args[3].strVal.size(),
                       0, (sockaddr*)&a, sizeof(a));
    if (r < 0) return err("udpSend: sendto failed");
    return Value::makeInt((int64_t)r);
}

static Value builtinUdpRecvFrom(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::Int)
        return err("udpRecvFrom: expected (fd, n)");
    int n = (int)args[1].intVal;
    std::string buf((size_t)n, '\0');
    sockaddr_in a;
    socklen_t al = sizeof(a);
    ssize_t r = recvfrom((int)args[0].intVal, &buf[0], (size_t)n, 0, (sockaddr*)&a, &al);
    if (r < 0) return err("udpRecvFrom: recvfrom failed");
    buf.resize((size_t)r);
    char host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &a.sin_addr, host, sizeof(host));
    return Value::makeArray({Value::makeString(buf),
        Value::makeString(std::string(host) + ":" + std::to_string(ntohs(a.sin_port)))});
}

void registerNetBuiltins(std::shared_ptr<Env> env) {
    auto reg = [&](const std::string& name, auto fn) {
        env->set(name, Value::makeBuiltin({fn}));
    };
    reg("netListen", builtinNetListen);
    reg("netPort", builtinNetPort);
    reg("netAccept", builtinNetAccept);
    reg("netConnect", builtinNetConnect);
    reg("netRead", builtinNetRead);
    reg("netWrite", builtinNetWrite);
    reg("netClose", builtinNetClose);
    reg("netSetTimeout", builtinNetSetTimeout);
    reg("netPeer", builtinNetPeer);
    reg("udpSocket", builtinUdpSocket);
    reg("udpBind", builtinUdpBind);
    reg("udpSend", builtinUdpSend);
    reg("udpRecvFrom", builtinUdpRecvFrom);
}

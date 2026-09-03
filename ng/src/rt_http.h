#pragma once
#include <string>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::unordered_map<std::string, std::string> headers; // lowercased keys
    std::string body;
    bool keepAlive = true;
    bool isUpgradeWs = false;
    std::string wsKey;
    bool complete = false;
};

// Incremental HTTP/1.1 head parser. Feed bytes, returns bytes-consumed for
// one complete head, or 0 if need more data, or -1 on hard error (>16KB head,
// >32 headers). Body NOT consumed here (Content-Length handled by caller;
// chunked deferred to 4.2).
struct HttpParser {
    static constexpr size_t kMaxHead = 16 * 1024;
    static constexpr size_t kMaxHeaders = 32;
    // Try parse one request head from buf[0..len). On success fills req and
    // returns head length. Returns 0 = need more, -1 = error.
    static long parseHead(const char* buf, size_t len, HttpRequest& req);
    static std::string lower(const std::string& s);
};

#include "rt_http.h"
#include <cctype>

std::string HttpParser::lower(const std::string& s) {
    std::string o = s;
    for (auto& c : o) c = (char)tolower((unsigned char)c);
    return o;
}

long HttpParser::parseHead(const char* buf, size_t len, HttpRequest& req) {
    if (len > kMaxHead + 4096) return -1;
    std::string view(buf, len);
    size_t he = view.find("\r\n\r\n");
    if (he == std::string::npos) {
        if (len > kMaxHead) return -1;
        return 0; // need more
    }
    size_t headLen = he + 4;
    // request line
    size_t p0 = view.find("\r\n");
    if (p0 == std::string::npos) return -1;
    std::string rl = view.substr(0, p0);
    size_t s1 = rl.find(' ');
    size_t s2 = rl.find(' ', s1 == std::string::npos ? 0 : s1 + 1);
    if (s1 == std::string::npos || s2 == std::string::npos) return -1;
    req.method = rl.substr(0, s1);
    std::string target = rl.substr(s1 + 1, s2 - s1 - 1);
    size_t q = target.find('?');
    if (q == std::string::npos) { req.path = target; req.query = ""; }
    else { req.path = target.substr(0, q); req.query = target.substr(q + 1); }
    // headers
    req.headers.clear();
    size_t pos = p0 + 2;
    size_t count = 0;
    std::string conn, upgrade, wskey, clen;
    while (pos < he) {
        size_t e = view.find("\r\n", pos);
        if (e == std::string::npos || e > he) break;
        if (e == pos) break;
        std::string line = view.substr(pos, e - pos);
        size_t c = line.find(':');
        if (c != std::string::npos) {
            std::string k = lower(line.substr(0, c));
            std::string v = line.substr(c + 1);
            size_t b = 0;
            while (b < v.size() && (v[b] == ' ' || v[b] == '\t')) b++;
            v = v.substr(b);
            if (++count > kMaxHeaders) return -1;
            req.headers[k] = v;
            if (k == "connection") conn = lower(v);
            else if (k == "upgrade") upgrade = lower(v);
            else if (k == "sec-websocket-key") wskey = v;
        }
        pos = e + 2;
    }
    req.keepAlive = (conn.find("close") == std::string::npos);
    req.isUpgradeWs = (upgrade == "websocket" && !wskey.empty());
    req.wsKey = wskey;
    req.body = "";
    req.complete = true;
    return (long)headLen;
}

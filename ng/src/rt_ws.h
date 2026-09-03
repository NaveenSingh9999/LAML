#pragma once
#include <string>
#include <cstdint>
#include <optional>

// RFC6455 server-side, no deps. Reuses nothing from str_builtins
// (handshake needs SHA-1, LAML only has SHA-256).
struct WsFrame {
    uint8_t opcode = 0x1; // 0x0 cont, 0x1 text, 0x2 binary, 0x8 close, 0x9 ping, 0xA pong
    std::string payload;
    bool fin = true;
};

struct WsCodec {
    static constexpr size_t kMaxFrame = 1024 * 1024; // 1MB cap -> close 1009
    static std::string acceptKey(const std::string& clientKey);
    // Server -> client (unmasked)
    static std::string encode(const std::string& payload, uint8_t opcode = 0x1);
    static std::string encodeClose(uint16_t code = 1000, const std::string& reason = "");
    // Client -> server. Returns nullopt if need more bytes.
    // Sets errClose=true when peer violates protocol (oversize, bad rsv, unmasked).
    // consumed = bytes to drop from buffer on success.
    static std::optional<WsFrame> decode(const char* data, size_t len, size_t& consumed, bool& errClose);
};

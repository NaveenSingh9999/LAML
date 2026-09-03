#include "str_builtins.h"
#include "env.h"
#include "value.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

static std::string hexAlphabet = "0123456789abcdef";

static Value builtinSlice(const std::vector<Value>& args) {
    if (args.size() < 2 || args[1].type != ValType::Int)
        return Value::makeError("slice: expected (string|array, start[, n])");
    int64_t n3 = -1;
    if (args.size() >= 3) {
        if (args[2].type != ValType::Int) return Value::makeError("slice: n must be int");
        n3 = args[2].intVal;
        if (n3 < 0) return Value::makeError("slice: n must be >= 0");
    }
    if (args[0].type == ValType::Array && args[0].arrVal) {
        const auto& v = *args[0].arrVal;
        int64_t start = args[1].intVal;
        if (start < 0 || start > (int64_t)v.size())
            return Value::makeError("slice: start out of bounds");
        int64_t n = (int64_t)v.size() - start;
        if (n3 >= 0) n = std::min(n3, n);
        return Value::makeArray(std::vector<Value>(v.begin() + (size_t)start,
                                                   v.begin() + (size_t)(start + n)));
    }
    if (args[0].type != ValType::String)
        return Value::makeError("slice: expected (string|array, start[, n])");
    const std::string& s = args[0].strVal;
    int64_t start = args[1].intVal;
    if (start < 0 || start > (int64_t)s.size())
        return Value::makeError("slice: start out of bounds");
    int64_t n = (int64_t)s.size() - start;
    if (args.size() >= 3) {
        if (args[2].type != ValType::Int) return Value::makeError("slice: n must be int");
        n = args[2].intVal;
        if (n < 0) return Value::makeError("slice: n must be >= 0");
        if (start + n > (int64_t)s.size()) n = (int64_t)s.size() - start;
    }
    return Value::makeString(s.substr((size_t)start, (size_t)n));
}

static Value builtinSplit(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String || args[1].type != ValType::String)
        return Value::makeError("split: expected (string, sep)");
    const std::string& s = args[0].strVal;
    const std::string& sep = args[1].strVal;
    std::vector<Value> out;
    if (sep.empty()) {
        for (char c : s) out.push_back(Value::makeString(std::string(1, c)));
        return Value::makeArray(out);
    }
    size_t pos = 0;
    while (true) {
        size_t found = s.find(sep, pos);
        if (found == std::string::npos) {
            out.push_back(Value::makeString(s.substr(pos)));
            break;
        }
        out.push_back(Value::makeString(s.substr(pos, found - pos)));
        pos = found + sep.size();
    }
    return Value::makeArray(out);
}

static Value builtinIndexOf(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String || args[1].type != ValType::String)
        return Value::makeError("indexOf: expected (string, needle)");
    size_t p = args[0].strVal.find(args[1].strVal);
    return Value::makeInt(p == std::string::npos ? -1 : (int64_t)p);
}

static Value builtinContains(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String || args[1].type != ValType::String)
        return Value::makeError("contains: expected (string, needle)");
    return Value::makeBool(args[0].strVal.find(args[1].strVal) != std::string::npos);
}

static Value builtinToInt(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("toInt: expected a string");
    try {
        size_t idx = 0;
        long long v = std::stoll(args[0].strVal, &idx);
        if (idx != args[0].strVal.size()) return Value::makeError("toInt: not an integer");
        return Value::makeInt((int64_t)v);
    } catch (...) {
        return Value::makeError("toInt: not an integer");
    }
}

static Value builtinToFloat(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("toFloat: expected a string");
    try {
        size_t idx = 0;
        double v = std::stod(args[0].strVal, &idx);
        if (idx != args[0].strVal.size()) return Value::makeError("toFloat: not a number");
        return Value::makeFloat(v);
    } catch (...) {
        return Value::makeError("toFloat: not a number");
    }
}

static Value builtinTrim(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("trim: expected a string");
    const std::string& s = args[0].strVal;
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) b++;
    while (e > b && std::isspace((unsigned char)s[e - 1])) e--;
    return Value::makeString(s.substr(b, e - b));
}

static Value builtinChr(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return Value::makeError("chr: expected an int");
    int64_t v = args[0].intVal;
    if (v < 0 || v > 255) return Value::makeError("chr: value out of 0-255");
    return Value::makeString(std::string(1, (char)v));
}

static Value builtinOrd(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String || args[0].strVal.empty())
        return Value::makeError("ord: expected a non-empty string");
    return Value::makeInt((unsigned char)args[0].strVal[0]);
}

static Value builtinHexEncode(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("hexEncode: expected a string");
    const std::string& s = args[0].strVal;
    std::string out;
    out.reserve(s.size() * 2);
    for (unsigned char c : s) {
        out += hexAlphabet[c >> 4];
        out += hexAlphabet[c & 0x0f];
    }
    return Value::makeString(out);
}

static Value builtinHexDecode(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("hexDecode: expected a string");
    const std::string& s = args[0].strVal;
    if (s.size() % 2 != 0) return Value::makeError("hexDecode: odd length");
    std::string out;
    out.reserve(s.size() / 2);
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < s.size(); i += 2) {
        int hi = nib(s[i]), lo = nib(s[i + 1]);
        if (hi < 0 || lo < 0) return Value::makeError("hexDecode: invalid hex");
        out += (char)((hi << 4) | lo);
    }
    return Value::makeString(out);
}

static const std::string b64Alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static Value builtinBase64(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("base64: expected a string");
    const std::string& in = args[0].strVal;
    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);
    for (size_t i = 0; i < in.size(); i += 3) {
        unsigned v = (unsigned char)in[i] << 16;
        if (i + 1 < in.size()) v |= (unsigned char)in[i + 1] << 8;
        if (i + 2 < in.size()) v |= (unsigned char)in[i + 2];
        out += b64Alphabet[(v >> 18) & 0x3f];
        out += b64Alphabet[(v >> 12) & 0x3f];
        out += (i + 1 < in.size()) ? b64Alphabet[(v >> 6) & 0x3f] : '=';
        out += (i + 2 < in.size()) ? b64Alphabet[v & 0x3f] : '=';
    }
    return Value::makeString(out);
}

static Value builtinRandomBytes(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int || args[0].intVal < 0)
        return Value::makeError("randomBytes: expected non-negative int");
    size_t n = (size_t)args[0].intVal;
    std::string out(n, '\0');
    std::ifstream ur("/dev/urandom", std::ios::binary);
    if (!ur.is_open()) return Value::makeError("randomBytes: /dev/urandom unavailable");
    ur.read(&out[0], (std::streamsize)n);
    if (ur.gcount() != (std::streamsize)n)
        return Value::makeError("randomBytes: short read");
    return Value::makeString(out);
}

static Value builtinSha256(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("sha256: expected a string");
    const std::string& msg = args[0].strVal;
    // FIPS 180-4 SHA-256, compact implementation
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    std::vector<uint8_t> data(msg.begin(), msg.end());
    uint64_t bitlen = (uint64_t)data.size() * 8;
    data.push_back(0x80);
    while (data.size() % 64 != 56) data.push_back(0x00);
    for (int i = 7; i >= 0; i--) data.push_back((uint8_t)(bitlen >> (i * 8)));
    uint32_t h[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };
    auto rotr = [](uint32_t x, int n) { return (x >> n) | (x << (32 - n)); };
    for (size_t i = 0; i < data.size(); i += 64) {
        uint32_t w[64];
        for (int j = 0; j < 16; j++)
            w[j] = ((uint32_t)data[i + j * 4] << 24) | ((uint32_t)data[i + j * 4 + 1] << 16) |
                   ((uint32_t)data[i + j * 4 + 2] << 8) | (uint32_t)data[i + j * 4 + 3];
        for (int j = 16; j < 64; j++) {
            uint32_t s0 = rotr(w[j - 15], 7) ^ rotr(w[j - 15], 18) ^ (w[j - 15] >> 3);
            uint32_t s1 = rotr(w[j - 2], 17) ^ rotr(w[j - 2], 19) ^ (w[j - 2] >> 10);
            w[j] = w[j - 16] + s0 + w[j - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int j = 0; j < 64; j++) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = hh + S1 + ch + K[j] + w[j];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;
            hh = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }
    std::string out;
    out.reserve(32);
    for (int i = 0; i < 8; i++) {
        out += (char)(h[i] >> 24);
        out += (char)(h[i] >> 16);
        out += (char)(h[i] >> 8);
        out += (char)h[i];
    }
    return Value::makeString(out);
}

void registerStrBuiltins(std::shared_ptr<Env> env) {
    auto reg = [&](const std::string& name, auto fn) {
        env->set(name, Value::makeBuiltin({fn}));
    };
    reg("slice", builtinSlice);
    reg("split", builtinSplit);
    reg("indexOf", builtinIndexOf);
    reg("contains", builtinContains);
    reg("toInt", builtinToInt);
    reg("toFloat", builtinToFloat);
    reg("trim", builtinTrim);
    reg("chr", builtinChr);
    reg("ord", builtinOrd);
    reg("hexEncode", builtinHexEncode);
    reg("hexDecode", builtinHexDecode);
    reg("base64", builtinBase64);
    reg("sha256", builtinSha256);
    reg("randomBytes", builtinRandomBytes);
}

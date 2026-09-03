#include "rt_json.h"
#include "env.h"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <sstream>
#include <iomanip>

namespace {
constexpr size_t kMaxDoc = 1024 * 1024;
constexpr int kMaxDepth = 128;

struct P {
    const char* s; size_t n, p = 0;
    std::string err;
    int depth = 0;
    bool eof() const { return p >= n; }
    char peek() const { return eof() ? 0 : s[p]; }
    void ws() { while (!eof() && (s[p]==' '||s[p]=='\t'||s[p]=='\n'||s[p]=='\r')) p++; }
    bool lit(const char* w) {
        size_t i = 0;
        while (w[i] && p + i < n && s[p+i]==w[i]) i++;
        if (!w[i]) { p += i; return true; }
        return false;
    }
    Value fail(const std::string& m) { if (err.empty()) err = m; return Value::makeError(m); }
    void appendUtf8(std::string& o, unsigned cp) {
        if (cp < 0x80) o += (char)cp;
        else if (cp < 0x800) { o += (char)(0xC0|(cp>>6)); o += (char)(0x80|(cp&63)); }
        else if (cp < 0x10000) { o += (char)(0xE0|(cp>>12)); o += (char)(0x80|((cp>>6)&63)); o += (char)(0x80|(cp&63)); }
        else { o += (char)(0xF0|(cp>>18)); o += (char)(0x80|((cp>>12)&63)); o += (char)(0x80|((cp>>6)&63)); o += (char)(0x80|(cp&63)); }
    }
    bool hex4(unsigned& out) {
        if (p + 4 > n) return false;
        unsigned v = 0;
        for (int i = 0; i < 4; i++) {
            char c = s[p+i];
            v <<= 4;
            if (c>='0'&&c<='9') v |= (unsigned)(c-'0');
            else if (c>='a'&&c<='f') v |= (unsigned)(c-'a'+10);
            else if (c>='A'&&c<='F') v |= (unsigned)(c-'A'+10);
            else return false;
        }
        p += 4; out = v; return true;
    }
    Value parseString() {
        // assumes opening quote consumed
        std::string o;
        while (!eof()) {
            char c = s[p++];
            if (c == '"') return Value::makeString(std::move(o));
            if (c == '\\') {
                if (eof()) break;
                char e = s[p++];
                switch (e) {
                case '"': o+='"'; break;
                case '\\': o+='\\'; break;
                case '/': o+='/'; break;
                case 'b': o+='\b'; break;
                case 'f': o+='\f'; break;
                case 'n': o+='\n'; break;
                case 'r': o+='\r'; break;
                case 't': o+='\t'; break;
                case 'u': {
                    unsigned cp;
                    if (!hex4(cp)) return fail("json: bad \\u escape");
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (p+2 <= n && s[p]=='\\' && s[p+1]=='u') {
                            p += 2; unsigned lo;
                            if (!hex4(lo)) return fail("json: bad low surrogate");
                            if (lo < 0xDC00 || lo > 0xDFFF) return fail("json: bad low surrogate");
                            cp = 0x10000 + ((cp-0xD800)<<10) + (lo-0xDC00);
                        } else return fail("json: lone surrogate");
                    } else if (cp >= 0xDC00 && cp <= 0xDFFF) return fail("json: lone surrogate");
                    appendUtf8(o, cp);
                    break;
                }
                default: return fail("json: bad escape");
                }
            } else o += c;
        }
        return fail("json: unterminated string");
    }
    Value parseValue() {
        if (++depth > kMaxDepth) { --depth; return fail("json: nesting too deep"); }
        ws();
        if (eof()) { --depth; return fail("json: unexpected end"); }
        char c = s[p];
        Value v;
        if (c == '"') { p++; v = parseString(); }
        else if (c == '{') { p++; v = parseObj(); }
        else if (c == '[') { p++; v = parseArr(); }
        else if (c == 't') { if (!lit("true")) v=fail("json: bad literal"); else v=Value::makeBool(true); }
        else if (c == 'f') { if (!lit("false")) v=fail("json: bad literal"); else v=Value::makeBool(false); }
        else if (c == 'n') { if (!lit("null")) v=fail("json: bad literal"); else v=Value(); }
        else if (c == '-' || (c >= '0' && c <= '9')) v = parseNum();
        else v = fail("json: unexpected character");
        --depth;
        return v;
    }
    Value parseNum() {
        size_t st = p;
        if (peek()=='-') p++;
        while (!eof() && isdigit((unsigned char)peek())) p++;
        bool isFloat = false;
        if (!eof() && peek()=='.') { isFloat=true; p++; while(!eof()&&isdigit((unsigned char)peek())) p++; }
        if (!eof() && (peek()=='e'||peek()=='E')) {
            isFloat=true; p++;
            if (!eof() && (peek()=='+'||peek()=='-')) p++;
            if (eof() || !isdigit((unsigned char)peek())) return fail("json: bad number");
            while(!eof()&&isdigit((unsigned char)peek())) p++;
        }
        std::string t(s+st, p-st);
        if (t=="-" || t.empty()) return fail("json: bad number");
        if (!isFloat) {
            char* e=nullptr;
            errno = 0;
            long long v = strtoll(t.c_str(), &e, 10);
            if (e && *e==0 && errno != ERANGE) return Value::makeInt((int64_t)v);
            isFloat = true; // overflow -> float
        }
        char* e=nullptr;
        double d = strtod(t.c_str(), &e);
        if (!e || *e!=0) return fail("json: bad number");
        return Value::makeFloat(d);
    }
    Value parseArr() {
        std::vector<Value> out;
        ws();
        if (!eof() && peek()==']') { p++; return Value::makeArray(out); }
        while (true) {
            Value v = parseValue();
            if (v.type == ValType::Error) return v; // LAML has no error literal: always a real failure
            out.push_back(v);
            ws();
            if (eof()) return fail("json: unterminated array");
            if (peek()==',') { p++; continue; }
            if (peek()==']') { p++; return Value::makeArray(out); }
            return fail("json: expected , or ]");
        }
    }
    Value parseObj() {
        auto e = std::make_shared<Env>();
        ws();
        if (!eof() && peek()=='}') { p++; return Value::makeObj(e); }
        while (true) {
            ws();
            if (eof() || peek()!='"') return fail("json: expected string key");
            p++;
            Value k = parseString();
            if (k.type==ValType::Error) return k;
            ws();
            if (eof() || peek()!=':') return fail("json: expected :");
            p++;
            Value v = parseValue();
            if (v.type==ValType::Error) return v; // always a real failure (no error literal in JSON)
            e->declare(k.strVal, v);
            ws();
            if (eof()) return fail("json: unterminated object");
            if (peek()==',') { p++; continue; }
            if (peek()=='}') { p++; return Value::makeObj(e); }
            return fail("json: expected , or }");
        }
    }
};

void strJson(std::string& o, const std::string& s) {
    o += '"';
    for (unsigned char c : s) {
        switch (c) {
        case '"': o += "\\\""; break;
        case '\\': o += "\\\\"; break;
        case '\b': o += "\\b"; break;
        case '\f': o += "\\f"; break;
        case '\n': o += "\\n"; break;
        case '\r': o += "\\r"; break;
        case '\t': o += "\\t"; break;
        default:
            if (c < 0x20) { char b[7]; snprintf(b,sizeof(b),"\\u%04x",c); o += b; }
            else o += (char)c;
        }
    }
    o += '"';
}
bool stringifyInto(std::string& o, const Value& v, int depth) {
    if (depth > kMaxDepth) return false;
    switch (v.type) {
    case ValType::Nil: o += "null"; return true;
    case ValType::Bool: o += v.boolVal ? "true" : "false"; return true;
    case ValType::Int: o += std::to_string(v.intVal); return true;
    case ValType::Float: {
        std::ostringstream os;
        os << std::setprecision(17) << v.floatVal;
        o += os.str(); return true;
    }
    case ValType::String: strJson(o, v.strVal); return true;
    case ValType::Error: strJson(o, v.errMsg); return true;
    case ValType::Array: {
        o += '[';
        if (v.arrVal) for (size_t i=0;i<v.arrVal->size();i++) {
            if (i) o += ',';
            if (!stringifyInto(o, (*v.arrVal)[i], depth+1)) return false;
        }
        o += ']'; return true;
    }
    case ValType::Obj: {
        o += '{'; bool first = true;
        if (v.objVal) {
            // Env store is unordered: document unordered keys. Lock briefly.
            std::lock_guard<std::mutex> l(v.objVal->env->mtx);
            for (auto& kv : v.objVal->env->store) {
                if (!first) o += ',';
                first = false;
                strJson(o, kv.first); o += ':';
                if (!stringifyInto(o, kv.second, depth+1)) return false;
            }
        }
        o += '}'; return true;
    }
    case ValType::Func: o += "\"<func>\""; return true;
    case ValType::Builtin: o += "\"<builtin>\""; return true;
    case ValType::Closc: o += "\"<closc>\""; return true;
    }
    return false;
}
}

Value RtJson::parse(const std::string& doc) {
    if (doc.size() > kMaxDoc) return Value::makeError("json: document too large");
    P p; p.s = doc.data(); p.n = doc.size();
    Value v = p.parseValue();
    if (v.type == ValType::Error) return v;
    p.ws();
    if (!p.eof()) return Value::makeError("json: trailing data");
    return v;
}

Value RtJson::stringify(const Value& v) {
    std::string o;
    o.reserve(64);
    if (!stringifyInto(o, v, 0)) return Value::makeError("json: nesting too deep");
    return Value::makeString(std::move(o));
}

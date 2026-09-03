#include "gen_builtins.h"
#include "env.h"
#include "value.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <random>
#include <string>
#include <vector>

static Value err(const std::string& m) { return Value::makeError(m); }
static bool isNum(const Value& v) { return v.type == ValType::Int || v.type == ValType::Float; }
static double num(const Value& v) { return v.type == ValType::Float ? v.floatVal : (double)v.intVal; }

// ---------- math ----------
static Value bAbs(const std::vector<Value>& a) {
    if (a.empty() || !isNum(a[0])) return err("abs: expected a number");
    if (a[0].type == ValType::Int) {
        if (a[0].intVal == INT64_MIN) return err("abs: overflow");
        return Value::makeInt(a[0].intVal < 0 ? -a[0].intVal : a[0].intVal);
    }
    return Value::makeFloat(std::fabs(a[0].floatVal));
}
static Value bMin(const std::vector<Value>& a) {
    if (a.size() < 2 || !isNum(a[0]) || !isNum(a[1])) return err("min: expected (a, b) numbers");
    if (a[0].type == ValType::Int && a[1].type == ValType::Int)
        return Value::makeInt(std::min(a[0].intVal, a[1].intVal));
    return Value::makeFloat(std::fmin(num(a[0]), num(a[1])));
}
static Value bMax(const std::vector<Value>& a) {
    if (a.size() < 2 || !isNum(a[0]) || !isNum(a[1])) return err("max: expected (a, b) numbers");
    if (a[0].type == ValType::Int && a[1].type == ValType::Int)
        return Value::makeInt(std::max(a[0].intVal, a[1].intVal));
    return Value::makeFloat(std::fmax(num(a[0]), num(a[1])));
}
static Value bFloor(const std::vector<Value>& a) {
    if (a.empty() || !isNum(a[0])) return err("floor: expected a number");
    if (a[0].type == ValType::Int) return Value::makeInt(a[0].intVal);
    return Value::makeFloat(std::floor(a[0].floatVal));
}
static Value bCeil(const std::vector<Value>& a) {
    if (a.empty() || !isNum(a[0])) return err("ceil: expected a number");
    if (a[0].type == ValType::Int) return Value::makeInt(a[0].intVal);
    return Value::makeFloat(std::ceil(a[0].floatVal));
}
static Value bSqrt(const std::vector<Value>& a) {
    if (a.empty() || !isNum(a[0])) return err("sqrt: expected a number");
    if (num(a[0]) < 0) return err("sqrt: negative");
    return Value::makeFloat(std::sqrt(num(a[0])));
}
static Value bPow(const std::vector<Value>& a) {
    if (a.size() < 2 || !isNum(a[0]) || !isNum(a[1])) return err("pow: expected (x, y) numbers");
    double r = std::pow(num(a[0]), num(a[1]));
    if (!std::isfinite(r)) return err("pow: out of range");
    return Value::makeFloat(r);
}
static Value bRandInt(const std::vector<Value>& a) {
    thread_local std::mt19937_64 rng{std::random_device{}()};
    int64_t lo = 0, hi = 0;
    if (a.size() == 1 && a[0].type == ValType::Int) { hi = a[0].intVal; }
    else if (a.size() >= 2 && a[0].type == ValType::Int && a[1].type == ValType::Int) {
        lo = a[0].intVal; hi = a[1].intVal;
    } else return err("randInt: expected randInt(n) or randInt(lo, hi)");
    if (hi < lo) return err("randInt: hi < lo");
    std::uniform_int_distribution<int64_t> d(lo, hi);
    return Value::makeInt(d(rng));
}

// ---------- arrays (in place, like append) ----------
static Value needArr(const std::vector<Value>& a, const char* name) {
    if (a.empty() || a[0].type != ValType::Array || !a[0].arrVal)
        return err(std::string(name) + ": expected (array, ...)");
    return Value();
}
static Value bPop(const std::vector<Value>& a) {
    if (Value e = needArr(a, "pop"); e.type == ValType::Error) return e;
    auto& v = *a[0].arrVal;
    if (v.empty()) return err("pop: empty array");
    Value back = v.back();
    v.pop_back();
    return back;
}
static Value bRemove(const std::vector<Value>& a) {
    if (Value e = needArr(a, "remove"); e.type == ValType::Error) return e;
    if (a.size() < 2 || a[1].type != ValType::Int) return err("remove: expected (array, index)");
    auto& v = *a[0].arrVal;
    int64_t i = a[1].intVal;
    if (i < 0 || i >= (int64_t)v.size()) return err("remove: index out of bounds");
    Value gone = v[(size_t)i];
    v.erase(v.begin() + (size_t)i);
    return gone;
}
static Value bReverse(const std::vector<Value>& a) {
    if (Value e = needArr(a, "reverse"); e.type == ValType::Error) return e;
    auto& v = *a[0].arrVal;
    std::reverse(v.begin(), v.end());
    return NIL;
}
static Value bSort(const std::vector<Value>& a) {
    if (Value e = needArr(a, "sort"); e.type == ValType::Error) return e;
    auto& v = *a[0].arrVal;
    // Numbers sort numerically, strings lexicographically, mixed by rendered form.
    std::stable_sort(v.begin(), v.end(), [](const Value& x, const Value& y) {
        bool xn = isNum(x), yn = isNum(y);
        if (xn && yn) return num(x) < num(y);
        if (x.type == ValType::String && y.type == ValType::String) return x.strVal < y.strVal;
        return x.inspect() < y.inspect();
    });
    return NIL;
}
static Value bJoin(const std::vector<Value>& a) {
    if (Value e = needArr(a, "join"); e.type == ValType::Error) return e;
    std::string sep = (a.size() >= 2 && a[1].type == ValType::String) ? a[1].strVal : "";
    std::string o;
    auto& v = *a[0].arrVal;
    for (size_t i = 0; i < v.size(); i++) {
        if (i) o += sep;
        o += (v[i].type == ValType::String) ? v[i].strVal : v[i].inspect();
    }
    return Value::makeString(std::move(o));
}

// ---------- strings ----------
static Value needStr(const std::vector<Value>& a, size_t n, const char* name) {
    if (a.size() < n) return err(std::string(name) + ": not enough args");
    for (size_t i = 0; i < n; i++)
        if (a[i].type != ValType::String) return err(std::string(name) + ": expected strings");
    return Value();
}
static Value bUpper(const std::vector<Value>& a) {
    if (Value e = needStr(a, 1, "upper"); e.type == ValType::Error) return e;
    std::string s = a[0].strVal;
    for (auto& c : s) c = (char)toupper((unsigned char)c);
    return Value::makeString(std::move(s));
}
static Value bLower(const std::vector<Value>& a) {
    if (Value e = needStr(a, 1, "lower"); e.type == ValType::Error) return e;
    std::string s = a[0].strVal;
    for (auto& c : s) c = (char)tolower((unsigned char)c);
    return Value::makeString(std::move(s));
}
static Value bStartsWith(const std::vector<Value>& a) {
    if (Value e = needStr(a, 2, "startsWith"); e.type == ValType::Error) return e;
    const auto& s = a[0].strVal;
    const auto& p = a[1].strVal;
    return Value::makeBool(s.size() >= p.size() && s.compare(0, p.size(), p) == 0);
}
static Value bEndsWith(const std::vector<Value>& a) {
    if (Value e = needStr(a, 2, "endsWith"); e.type == ValType::Error) return e;
    const auto& s = a[0].strVal;
    const auto& p = a[1].strVal;
    return Value::makeBool(s.size() >= p.size() && s.compare(s.size() - p.size(), p.size(), p) == 0);
}
static Value bReplace(const std::vector<Value>& a) {
    if (Value e = needStr(a, 3, "replace"); e.type == ValType::Error) return e;
    const auto& s = a[0].strVal;
    const auto& from = a[1].strVal;
    const auto& to = a[2].strVal;
    if (from.empty()) return Value::makeString(s);
    std::string o;
    size_t pos = 0;
    while (true) {
        size_t f = s.find(from, pos);
        if (f == std::string::npos) { o += s.substr(pos); break; }
        o += s.substr(pos, f - pos);
        o += to;
        pos = f + from.size();
    }
    return Value::makeString(std::move(o));
}
static Value bRepeat(const std::vector<Value>& a) {
    if (a.size() < 2 || a[0].type != ValType::String || a[1].type != ValType::Int)
        return err("repeat: expected (string, n)");
    if (a[1].intVal < 0 || a[1].intVal > 1000000) return err("repeat: n out of range 0..1000000");
    std::string o;
    o.reserve(a[0].strVal.size() * (size_t)a[1].intVal);
    for (int64_t i = 0; i < a[1].intVal; i++) o += a[0].strVal;
    return Value::makeString(std::move(o));
}

// ---------- objects ----------
static Value bKeys(const std::vector<Value>& a) {
    if (a.empty() || a[0].type != ValType::Obj || !a[0].objVal)
        return err("keys: expected an object");
    std::vector<Value> out;
    std::lock_guard<std::mutex> l(a[0].objVal->env->mtx);
    for (auto& kv : a[0].objVal->env->store) out.push_back(Value::makeString(kv.first));
    return Value::makeArray(out);
}
static Value bHas(const std::vector<Value>& a) {
    if (a.size() < 2 || a[0].type != ValType::Obj || !a[0].objVal || a[1].type != ValType::String)
        return err("has: expected (object, key)");
    auto v = a[0].objVal->env->get(a[1].strVal);
    return Value::makeBool(v.has_value());
}

// ---------- conversion ----------
static Value bString(const std::vector<Value>& a) {
    if (a.empty()) return Value::makeString("");
    if (a[0].type == ValType::String) return Value::makeString(a[0].strVal);
    return Value::makeString(a[0].inspect());
}

// ---------- fs (small, safe: no shell, paths as given) ----------
static Value bFileExists(const std::vector<Value>& a) {
    if (a.empty() || a[0].type != ValType::String) return err("fileExists: expected a path");
    std::ifstream f(a[0].strVal, std::ios::binary);
    return Value::makeBool(f.good());
}
static Value bFileDelete(const std::vector<Value>& a) {
    if (a.empty() || a[0].type != ValType::String) return err("fileDelete: expected a path");
    if (std::remove(a[0].strVal.c_str()) != 0) return err("fileDelete: cannot delete " + a[0].strVal);
    return NIL;
}
static Value bAppendFile(const std::vector<Value>& a) {
    if (a.size() < 2 || a[0].type != ValType::String || a[1].type != ValType::String)
        return err("appendFile: expected (path, data)");
    std::ofstream f(a[0].strVal, std::ios::binary | std::ios::app);
    if (!f.is_open()) return err("appendFile: cannot open " + a[0].strVal);
    f << a[1].strVal;
    return NIL;
}

// ---------- assert ----------
static Value bAssert(const std::vector<Value>& a) {
    if (a.empty()) return err("assert: expected (cond[, msg])");
    if (a[0].isTruthy()) return NIL;
    std::string msg = "assertion failed";
    if (a.size() >= 2 && a[1].type == ValType::String) msg = a[1].strVal;
    return err("assert: " + msg);
}

void registerGenBuiltins(std::shared_ptr<Env> env) {
    auto reg = [&](const std::string& n, auto fn) { env->set(n, Value::makeBuiltin({fn})); };
    reg("abs", bAbs);
    reg("min", bMin);
    reg("max", bMax);
    reg("floor", bFloor);
    reg("ceil", bCeil);
    reg("sqrt", bSqrt);
    reg("pow", bPow);
    reg("randInt", bRandInt);
    reg("pop", bPop);
    reg("remove", bRemove);
    reg("reverse", bReverse);
    reg("sort", bSort);
    reg("join", bJoin);
    reg("upper", bUpper);
    reg("lower", bLower);
    reg("startsWith", bStartsWith);
    reg("endsWith", bEndsWith);
    reg("replace", bReplace);
    reg("repeat", bRepeat);
    reg("keys", bKeys);
    reg("has", bHas);
    reg("string", bString);
    reg("fileExists", bFileExists);
    reg("fileDelete", bFileDelete);
    reg("appendFile", bAppendFile);
    reg("assert", bAssert);
}

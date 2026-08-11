#include "builtins.h"
#include "sys_builtins.h"
#include "str_builtins.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>

static Value builtinSay(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].inspect();
    }
    std::cout << std::endl;
    return NIL;
}

static Value builtinPrint(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].inspect();
    }
    return NIL;
}

static Value builtinType(const std::vector<Value>& args) {
    if (args.empty()) return Value::makeString("nil");
    switch (args[0].type) {
    case ValType::Nil: return Value::makeString("nil");
    case ValType::Int: return Value::makeString("int");
    case ValType::Float: return Value::makeString("float");
    case ValType::String: return Value::makeString("string");
    case ValType::Bool: return Value::makeString("bool");
    case ValType::Array: return Value::makeString("array");
    case ValType::Func: return Value::makeString("func");
    case ValType::Builtin: return Value::makeString("builtin");
    case ValType::Obj: return Value::makeString("obj");
    case ValType::Closc: return Value::makeString("closc");
    case ValType::Error: return Value::makeString("error");
    }
    return Value::makeString("unknown");
}

static Value builtinLen(const std::vector<Value>& args) {
    if (args.empty()) return Value::makeInt(0);
    if (args[0].type == ValType::String) return Value::makeInt(args[0].strVal.size());
    if (args[0].type == ValType::Array && args[0].arrVal) return Value::makeInt(args[0].arrVal->size());
    return Value::makeError("len: unsupported type");
}

static Value builtinSleep(const std::vector<Value>& args) {
    if (args.empty()) return NIL;
    int64_t ms = 1000;
    if (args[0].type == ValType::Int) ms = args[0].intVal;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return NIL;
}

static Value builtinClock(const std::vector<Value>&) {
    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return Value::makeInt(ms);
}

static Value builtinReadLine(const std::vector<Value>&) {
    std::string line;
    if (!std::getline(std::cin, line)) return NIL;
    return Value::makeString(line);
}

static Value builtinReadFile(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("readFile: expected a filename string");
    std::ifstream file(args[0].strVal);
    if (!file.is_open())
        return Value::makeError("readFile: cannot open " + args[0].strVal);
    std::stringstream ss;
    ss << file.rdbuf();
    return Value::makeString(ss.str());
}

void registerBuiltins(std::shared_ptr<Env> env) {
    auto reg = [&](const std::string& name, auto fn) {
        env->set(name, Value::makeBuiltin({fn}));
    };

    reg("say", builtinSay);
    reg("print", builtinPrint);
    reg("type", builtinType);
    reg("len", builtinLen);
    reg("sleep", builtinSleep);
    reg("clock", builtinClock);
    reg("readLine", builtinReadLine);
    reg("readFile", builtinReadFile);

    registerSysBuiltins(env);
    registerStrBuiltins(env);
}

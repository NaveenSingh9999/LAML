#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <sstream>

struct Env;
struct ASTNode;

enum class ValType {
    Nil, Int, Float, String, Bool, Array,
    Func, Builtin, Obj, Closc, Error
};

struct FuncData {
    std::vector<std::string> params;
    ASTNode* body = nullptr;
    std::shared_ptr<Env> closure;
};

struct CloscData {
    std::string name;
    int priority = 0;
    ASTNode* body = nullptr;
    std::shared_ptr<Env> closure;
};

struct BuiltinData {
    std::function<struct Value(std::vector<struct Value>)> fn;
};

struct ObjData {
    std::shared_ptr<Env> env;
};

struct Value {
    ValType type = ValType::Nil;
    int64_t intVal = 0;
    double floatVal = 0.0;
    bool boolVal = false;
    std::string strVal;
    std::shared_ptr<std::vector<Value>> arrVal;
    std::unique_ptr<FuncData> funcVal;
    std::unique_ptr<CloscData> closcVal;
    std::unique_ptr<BuiltinData> builtinVal;
    std::unique_ptr<ObjData> objVal;
    std::string errMsg;
    bool returning = false;

    Value() : type(ValType::Nil) {}
    Value(const Value& other) { copyFrom(other); }
    Value& operator=(const Value& other) { if (this != &other) copyFrom(other); return *this; }
    Value(Value&&) = default;
    Value& operator=(Value&&) = default;

    void copyFrom(const Value& other) {
        type = other.type;
        intVal = other.intVal;
        floatVal = other.floatVal;
        boolVal = other.boolVal;
        strVal = other.strVal;
        errMsg = other.errMsg;
        returning = other.returning;
        if (other.arrVal) arrVal = std::make_shared<std::vector<Value>>(*other.arrVal);
        else arrVal.reset();
        if (other.funcVal) funcVal = std::make_unique<FuncData>(*other.funcVal);
        else funcVal.reset();
        if (other.closcVal) closcVal = std::make_unique<CloscData>(*other.closcVal);
        else closcVal.reset();
        if (other.builtinVal) builtinVal = std::make_unique<BuiltinData>(*other.builtinVal);
        else builtinVal.reset();
        if (other.objVal) objVal = std::make_unique<ObjData>(*other.objVal);
        else objVal.reset();
    }

    static Value makeInt(int64_t v) { Value r; r.type = ValType::Int; r.intVal = v; return r; }
    static Value makeFloat(double v) { Value r; r.type = ValType::Float; r.floatVal = v; return r; }
    static Value makeString(const std::string& v) { Value r; r.type = ValType::String; r.strVal = v; return r; }
    static Value makeBool(bool v) { Value r; r.type = ValType::Bool; r.boolVal = v; return r; }
    static Value makeArray(const std::vector<Value>& v) { Value r; r.type = ValType::Array; r.arrVal = std::make_shared<std::vector<Value>>(v); return r; }
    static Value makeError(const std::string& m) { Value r; r.type = ValType::Error; r.errMsg = m; return r; }
    static Value makeFunc(FuncData d) { Value r; r.type = ValType::Func; r.funcVal = std::make_unique<FuncData>(std::move(d)); return r; }
    static Value makeBuiltin(BuiltinData d) { Value r; r.type = ValType::Builtin; r.builtinVal = std::make_unique<BuiltinData>(std::move(d)); return r; }
    static Value makeObj(std::shared_ptr<Env> e) { Value r; r.type = ValType::Obj; r.objVal = std::make_unique<ObjData>(ObjData{std::move(e)}); return r; }
    static Value makeClosc(CloscData d) { Value r; r.type = ValType::Closc; r.closcVal = std::make_unique<CloscData>(std::move(d)); return r; }
    static Value makeNil() { return Value(); }

    std::string inspect() const;
    bool isTruthy() const;
    Value copy() const;
};

const Value NIL = Value::makeNil();
const Value TRUE_V = Value::makeBool(true);
const Value FALSE_V = Value::makeBool(false);

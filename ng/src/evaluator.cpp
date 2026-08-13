#include "evaluator.h"
#include "builtins.h"
#include "closc.h"
#include "scheduler.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <limits>
#include <string>
#include <vector>

static Value error(const std::string& msg) {
    return Value::makeError(msg);
}

Evaluator* Evaluator::current_ = nullptr;

bool isError(const Value& v) { return v.type == ValType::Error; }

Value Evaluator::eval(const ASTNode& node, std::shared_ptr<Env> env) {
    switch (node.kind) {
    case ASTNode::Kind::Program:
        return evalProgram(node, env);
    case ASTNode::Kind::Block:
        return evalBlock(node, env);
    case ASTNode::Kind::ExprStmt:
        if (!node.children.empty())
            return eval(node.children[0], env);
        return NIL;
    case ASTNode::Kind::ValDecl: {
        Value val = NIL;
        if (!node.children.empty()) {
            val = eval(node.children[0], env);
            if (val.type == ValType::Error) return val;
        }
        env->declare(node.strVal, val, true); // val bindings are read-only
        return NIL;
    }
    case ASTNode::Kind::LetDecl: {
        Value val = NIL;
        if (!node.children.empty()) {
            val = eval(node.children[0], env);
            if (val.type == ValType::Error) return val;
        }
        env->declare(node.strVal, val, false);
        return NIL;
    }
    case ASTNode::Kind::Say:
        return evalSay(node, env);
    case ASTNode::Kind::If:
        return evalIf(node, env);
    case ASTNode::Kind::While:
        return evalWhile(node, env);
    case ASTNode::Kind::Loop:
        return evalLoop(node, env);
    case ASTNode::Kind::RangeLoop:
        return evalLoop(node, env);
    case ASTNode::Kind::ForIn:
        return evalForIn(node, env);
    case ASTNode::Kind::Break: {
        Value v = NIL;
        v.breaking = true;
        return v;
    }
    case ASTNode::Kind::Continue: {
        Value v = NIL;
        v.continuing = true;
        return v;
    }
    case ASTNode::Kind::Return: {
        Value val = NIL;
        if (!node.children.empty())
            val = eval(node.children[0], env);
        val.returning = true;
        return val;
    }
    case ASTNode::Kind::Ident:
        return evalIdent(node, env);
    case ASTNode::Kind::IntLit:
        return Value::makeInt(node.intVal);
    case ASTNode::Kind::FloatLit:
        return Value::makeFloat(node.floatVal);
    case ASTNode::Kind::StrLit:
        return Value::makeString(node.strVal);
    case ASTNode::Kind::BoolLit:
        return Value::makeBool(node.boolVal);
    case ASTNode::Kind::Prefix:
        return evalPrefix(node, env);
    case ASTNode::Kind::Postfix:
        return evalPostfix(node, env);
    case ASTNode::Kind::Infix:
        return evalInfix(node, env);
    case ASTNode::Kind::Call:
        return evalCall(node, env);
    case ASTNode::Kind::Index:
        return evalIndex(node, env);
    case ASTNode::Kind::Dot:
        return evalDot(node, env);
    case ASTNode::Kind::ObjLit:
        return evalObjLit(node, env);
    case ASTNode::Kind::ArrayLit: {
        std::vector<Value> elems;
        for (const auto& child : node.children) {
            Value v = eval(child, env);
            if (isError(v)) return v;
            elems.push_back(v);
        }
        return Value::makeArray(elems);
    }
    case ASTNode::Kind::FuncDecl: {
        FuncData fd;
        for (size_t i = 0; i + 1 < node.children.size(); i++) {
            fd.params.push_back(node.children[i].strVal);
        }
        if (!node.children.empty())
            fd.body = const_cast<ASTNode*>(&node.children.back());
        fd.closure = env;
        Value fn = Value::makeFunc(std::move(fd));
        if (!node.strVal.empty())
            env->declare(node.strVal, fn);
        return fn;
    }
    case ASTNode::Kind::Assign: {
        if (node.children.size() >= 2) {
            const auto& target = node.children[0];
            Value val = eval(node.children[1], env);
            if (isError(val)) return val;
            if (target.kind == ASTNode::Kind::Ident) {
                env->set(target.strVal, val);
                return val;
            }
        }
        return error("invalid assignment target");
    }
    case ASTNode::Kind::CloscDecl: {
        CloscData cd;
        cd.name = node.strVal.empty() ? "closc" : node.strVal;
        cd.priority = 5;
        if (!node.children.empty())
            cd.body = const_cast<ASTNode*>(&node.children[0]);
        cd.closure = env;
        auto closure = env;  // copy BEFORE makeClosc moves cd
        const ASTNode* body = cd.body;
        const std::string cname = cd.name;
        Value cv = Value::makeClosc(std::move(cd));
        env->set(cname, cv);

        CloscManager::instance().registerSection(
            cname, 5,
            [this, closure, body]() {
                auto localEnv = std::make_shared<Env>(closure);
                if (body) {
                    this->eval(*body, localEnv);
                }
            }
        );
        return NIL;
    }
    default:
        return NIL;
    }
}

Value Evaluator::evalProgram(const ASTNode& node, std::shared_ptr<Env> env) {
    Value result = NIL;
    for (const auto& stmt : node.children) {
        result = eval(stmt, env);
        if (result.type == ValType::Error) return result;
    }
    return result;
}

Value Evaluator::evalBlock(const ASTNode& node, std::shared_ptr<Env> env) {
    Value result = NIL;
    auto blockEnv = std::make_shared<Env>(env);
    for (const auto& stmt : node.children) {
        result = eval(stmt, blockEnv);
        if (result.type == ValType::Error) return result;
        if (result.returning) return result;
    }
    return result;
}

Value Evaluator::evalIdent(const ASTNode& node, std::shared_ptr<Env> env) {
    auto val = env->get(node.strVal);
    if (val) return *val;
    return error("undefined: " + node.strVal);
}

Value Evaluator::evalSay(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.empty()) {
        std::cout << std::endl;
        return NIL;
    }
    Value val = eval(node.children[0], env);
    if (isError(val)) return val;
    std::cout << val.inspect() << std::endl;
    return val;
}

Value Evaluator::evalIf(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.size() < 2) return NIL;
    Value cond = eval(node.children[0], env);
    if (isError(cond)) return cond;
    if (cond.isTruthy()) {
        return eval(node.children[1], env);
    } else if (node.children.size() >= 3) {
        return eval(node.children[2], env);
    }
    return NIL;
}

Value Evaluator::evalWhile(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.size() < 2) return NIL;
    Value result = NIL;
    size_t maxIters = 10'000'000;
    size_t iter = 0;
    while (true) {
        if (iter++ >= maxIters)
            return error("loop exceeded max iterations");
        Value cond = eval(node.children[0], env);
        if (isError(cond)) return cond;
        if (!cond.isTruthy()) break;
        result = eval(node.children[1], env);
        if (result.type == ValType::Error) return result;
        if (result.returning) return result;
    }
    return result;
}

Value Evaluator::evalLoop(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.kind == ASTNode::Kind::RangeLoop) {
        // loop var in start to end
        if (node.children.size() < 2) return NIL;
        Value start = eval(node.children[0], env);
        if (isError(start)) return start;
        Value end = eval(node.children[1], env);
        if (isError(end)) return end;
        if (start.type != ValType::Int || end.type != ValType::Int)
            return error("range loop requires integers");
        auto loopEnv = std::make_shared<Env>(env);
        Value result = NIL;
        size_t maxIters = 10'000'000;
        size_t iter = 0;
        for (int64_t i = start.intVal; i <= end.intVal; i++) {
            if (iter++ >= maxIters)
                return error("loop exceeded max iterations");
            loopEnv->set(node.strVal, Value::makeInt(i));
            if (node.children.size() >= 3)
                result = eval(node.children[2], loopEnv);
            else
                result = eval(node.children[1], loopEnv);
            if (result.type == ValType::Error) return result;
            if (result.returning) return result;
        }
        return result;
    }
    return evalWhile(node, env);
}

Value Evaluator::evalForIn(const ASTNode& node, std::shared_ptr<Env> env) {
    // for x in <start|collection> [to <end>] { body }
    if (node.children.empty()) return NIL;
    auto loopEnv = std::make_shared<Env>(env);
    Value result = NIL;
    size_t maxIters = 10'000'000;
    size_t iter = 0;
    // Returns false when the loop must stop (error, return, or break).
    auto runBody = [&](Value item) -> bool {
        if (iter++ >= maxIters) {
            result = error("loop exceeded max iterations");
            return false;
        }
        loopEnv->set(node.strVal, item);
        result = eval(node.children.back(), loopEnv);
        if (result.type == ValType::Error) return false;
        if (result.returning) return false;
        if (result.breaking) { result.breaking = false; result = NIL; return false; }
        if (result.continuing) { result.continuing = false; result = NIL; return true; }
        return true;
    };

    if (node.children.size() == 3) {
        // for x in start to end { body }
        Value start = eval(node.children[0], env);
        if (isError(start)) return start;
        Value end = eval(node.children[1], env);
        if (isError(end)) return end;
        if (start.type != ValType::Int || end.type != ValType::Int)
            return error("for-in range requires integers");
        for (int64_t i = start.intVal; i <= end.intVal; i++) {
            if (!runBody(Value::makeInt(i))) break;
        }
        return result;
    }

    // for x in <array|string> { body }
    Value coll = eval(node.children[0], env);
    if (isError(coll)) return coll;
    if (coll.type == ValType::Array && coll.arrVal) {
        for (const auto& item : *coll.arrVal) {
            if (!runBody(item)) break;
        }
        return result;
    }
    if (coll.type == ValType::String) {
        for (unsigned char c : coll.strVal) {
            if (!runBody(Value::makeInt(static_cast<int64_t>(c)))) break;
        }
        return result;
    }
    return error("for-in: expected array or string");
}

Value Evaluator::evalPostfix(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.empty()) return error("postfix needs operand");
    if (node.children[0].kind != ASTNode::Kind::Ident)
        return error("++/-- requires a variable");
    const std::string& name = node.children[0].strVal;
    auto cur = env->get(name);
    if (!cur) return error("undefined: " + name);
    if (cur->type != ValType::Int)
        return error("++/-- requires an integer variable");
    int64_t v = cur->intVal;
    Value nv = Value::makeInt(node.strVal == "++" ? v + 1 : v - 1);
    env->set(name, nv);
    return nv;
}

Value Evaluator::evalObjLit(const ASTNode& node, std::shared_ptr<Env> env) {
    auto objEnv = std::make_shared<Env>();
    for (size_t i = 0; i + 1 < node.children.size(); i += 2) {
        Value val = eval(node.children[i + 1], env);
        if (isError(val)) return val;
        objEnv->declare(node.children[i].strVal, val);
    }
    return Value::makeObj(objEnv);
}

Value Evaluator::evalInfix(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.size() < 2) return error("infix needs 2 operands");
    Value left = eval(node.children[0], env);
    if (isError(left)) return left;
    Value right = eval(node.children[1], env);
    if (isError(right)) return right;

    const std::string& op = node.strVal;

    // Assignment
    if (op == "=") {
        if (node.children[0].kind == ASTNode::Kind::Ident) {
            env->set(node.children[0].strVal, right);
            return right;
        }
        if (node.children[0].kind == ASTNode::Kind::Index) {
            const auto& idx = node.children[0];
            if (idx.children.empty()) return error("invalid array index");
            Value arr = eval(idx.children[0], env);
            if (isError(arr)) return arr;
            if (arr.type != ValType::Array || !arr.arrVal)
                return error("cannot index non-array");
            Value index = eval(idx.children[1], env);
            if (isError(index)) return index;
            if (index.type != ValType::Int)
                return error("array index must be an integer");
            if (index.intVal < 0 || (size_t)index.intVal >= arr.arrVal->size())
                return error("index out of bounds");
            (*arr.arrVal)[(size_t)index.intVal] = right;
            return right;
        }
        return error("cannot assign to non-identifier");
    }

    // Arithmetic
    if (left.type == ValType::Int && right.type == ValType::Int) {
        if (op == "+") return checkedAdd(left.intVal, right.intVal);
        if (op == "-") return checkedSub(left.intVal, right.intVal);
        if (op == "*") return checkedMul(left.intVal, right.intVal);
        if (op == "/") {
            if (right.intVal == 0) return error("division by zero");
            return Value::makeInt(left.intVal / right.intVal);
        }
        if (op == "%") {
            if (right.intVal == 0) return error("modulo by zero");
            return Value::makeInt(left.intVal % right.intVal);
        }
        if (op == "<") return Value::makeBool(left.intVal < right.intVal);
        if (op == ">") return Value::makeBool(left.intVal > right.intVal);
        if (op == "<=") return Value::makeBool(left.intVal <= right.intVal);
        if (op == ">=") return Value::makeBool(left.intVal >= right.intVal);
        if (op == "==") return Value::makeBool(left.intVal == right.intVal);
        if (op == "!=") return Value::makeBool(left.intVal != right.intVal);
    }

    if (left.type == ValType::Float && right.type == ValType::Float) {
        if (op == "+") return Value::makeFloat(left.floatVal + right.floatVal);
        if (op == "-") return Value::makeFloat(left.floatVal - right.floatVal);
        if (op == "*") return Value::makeFloat(left.floatVal * right.floatVal);
        if (op == "/") {
            if (right.floatVal == 0.0) return error("division by zero");
            return Value::makeFloat(left.floatVal / right.floatVal);
        }
        if (op == "<") return Value::makeBool(left.floatVal < right.floatVal);
        if (op == ">") return Value::makeBool(left.floatVal > right.floatVal);
        if (op == "==") return Value::makeBool(left.floatVal == right.floatVal);
        if (op == "!=") return Value::makeBool(left.floatVal != right.floatVal);
    }

    // Mixed int/float
    if ((left.type == ValType::Int || left.type == ValType::Float) &&
        (right.type == ValType::Int || right.type == ValType::Float)) {
        double l = left.type == ValType::Float ? left.floatVal : (double)left.intVal;
        double r = right.type == ValType::Float ? right.floatVal : (double)right.intVal;
        if (op == "+") return Value::makeFloat(l + r);
        if (op == "-") return Value::makeFloat(l - r);
        if (op == "*") return Value::makeFloat(l * r);
        if (op == "/") {
            if (r == 0.0) return error("division by zero");
            return Value::makeFloat(l / r);
        }
    }

    // String concat
    if (left.type == ValType::String || right.type == ValType::String) {
        if (op == "+") return Value::makeString(left.inspect() + right.inspect());
    }
    if (left.type == ValType::String && right.type == ValType::String) {
        if (op == "==") return Value::makeBool(left.strVal == right.strVal);
        if (op == "!=") return Value::makeBool(left.strVal != right.strVal);
        if (op == "+") return Value::makeString(left.strVal + right.strVal);
    }

    // Boolean
    if (op == "&&") return Value::makeBool(left.isTruthy() && right.isTruthy());
    if (op == "||") return Value::makeBool(left.isTruthy() || right.isTruthy());
    if (op == "==") return Value::makeBool(left.inspect() == right.inspect());
    if (op == "!=") return Value::makeBool(left.inspect() != right.inspect());

    return error("unknown operator: " + op);
}

Value Evaluator::evalPrefix(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.empty()) return error("prefix needs operand");
    Value right = eval(node.children[0], env);
    if (isError(right)) return right;

    if (node.strVal == "-") {
        if (right.type == ValType::Int) return Value::makeInt(-right.intVal);
        if (right.type == ValType::Float) return Value::makeFloat(-right.floatVal);
        return error("cannot negate");
    }
    if (node.strVal == "!") return Value::makeBool(!right.isTruthy());
    return error("unknown prefix: " + node.strVal);
}

Value Evaluator::evalCall(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.empty()) return error("call needs callee");
    Value callee = eval(node.children[0], env);
    if (isError(callee)) return callee;

    std::vector<Value> args;
    for (size_t i = 1; i < node.children.size(); i++) {
        Value arg = eval(node.children[i], env);
        // Don't propagate errors from args — let the callee handle them
        args.push_back(arg);
    }
    return applyFunc(callee, args);
}

Value Evaluator::evalIndex(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.size() < 2) return error("index needs 2 operands");
    Value left = eval(node.children[0], env);
    if (isError(left)) return left;
    Value idx = eval(node.children[1], env);
    if (isError(idx)) return idx;

    if (left.type == ValType::Array && idx.type == ValType::Int && left.arrVal) {
        int64_t i = idx.intVal;
        if (i < 0 || i >= (int64_t)left.arrVal->size())
            return error("index out of bounds");
        return (*left.arrVal)[i];
    }
    if (left.type == ValType::String && idx.type == ValType::Int) {
        int64_t i = idx.intVal;
        if (i < 0 || i >= (int64_t)left.strVal.size())
            return error("index out of bounds");
        auto ch = static_cast<unsigned char>(left.strVal[i]);
        return Value::makeInt(static_cast<int64_t>(ch));
    }
    return error("cannot index");
}

Value Evaluator::evalDot(const ASTNode& node, std::shared_ptr<Env> env) {
    if (node.children.size() < 2) return error("dot needs 2 operands");
    Value left = eval(node.children[0], env);
    if (isError(left)) return left;
    const std::string& prop = node.children[1].strVal;

    if (left.type == ValType::Obj && left.objVal) {
        auto val = left.objVal->env->get(prop);
        if (val) return *val;
        return error("no property: " + prop);
    }
    return error("cannot access property");
}

Value Evaluator::applyFunc(const Value& fn, const std::vector<Value>& args) {
    if (fn.type == ValType::Builtin && fn.builtinVal) {
        return fn.builtinVal->fn(args);
    }
    if (fn.type == ValType::Func && fn.funcVal) {
        auto localEnv = std::make_shared<Env>(fn.funcVal->closure);
        const auto& params = fn.funcVal->params;
        if (args.size() != params.size())
            return error("expected " + std::to_string(params.size()) +
                         " args, got " + std::to_string(args.size()));
        for (size_t i = 0; i < params.size(); i++) {
            localEnv->set(params[i], args[i]);
        }
        if (fn.funcVal->body) {
            Value result = eval(*fn.funcVal->body, localEnv);
            result.returning = false; // clear return flag for caller
            return result;
        }
        return NIL;
    }
    return error("not callable: " + fn.inspect());
}

// Checked arithmetic with overflow detection
Value Evaluator::checkedAdd(int64_t a, int64_t b) {
    if ((b > 0 && a > std::numeric_limits<int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<int64_t>::min() - b))
        return error("integer overflow");
    return Value::makeInt(a + b);
}

Value Evaluator::checkedSub(int64_t a, int64_t b) {
    if ((b > 0 && a < std::numeric_limits<int64_t>::min() + b) ||
        (b < 0 && a > std::numeric_limits<int64_t>::max() + b))
        return error("integer overflow");
    return Value::makeInt(a - b);
}

Value Evaluator::checkedMul(int64_t a, int64_t b) {
    if (a > 0 && b > 0 && a > std::numeric_limits<int64_t>::max() / b)
        return error("integer overflow");
    if (a < 0 && b < 0 && a < std::numeric_limits<int64_t>::max() / b)
        return error("integer overflow");
    if (a > 0 && b < 0 && b < std::numeric_limits<int64_t>::min() / a)
        return error("integer overflow");
    if (a < 0 && b > 0 && a < std::numeric_limits<int64_t>::min() / b)
        return error("integer overflow");
    return Value::makeInt(a * b);
}

Value Evaluator::checkedDiv(int64_t a, int64_t b) {
    if (b == 0) return error("division by zero");
    if (a == std::numeric_limits<int64_t>::min() && b == -1)
        return error("integer overflow");
    return Value::makeInt(a / b);
}

Value Evaluator::checkedMod(int64_t a, int64_t b) {
    if (b == 0) return error("modulo by zero");
    return Value::makeInt(a % b);
}

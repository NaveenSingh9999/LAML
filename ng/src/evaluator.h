#pragma once
#include "value.h"
#include "ast.h"
#include "env.h"
#include <memory>
#include <string>
#include <vector>

// Lvalue handle: keeps the backing container (array/object) alive while the
// slot pointer is in use, so it never dangles.
struct LValueRef {
    Value keeper1;
    Value keeper2;
    Value* slot = nullptr;
};

class Evaluator {
public:
    explicit Evaluator(std::shared_ptr<Env> global)
        : global(std::move(global)) { current() = this; }

    Value eval(const ASTNode& node, std::shared_ptr<Env> env);

    // Public so builtins (map/filter) can call first-class functions.
    Value applyFunc(const Value& fn, const std::vector<Value>& args);

    // Public so builtins (try/import) can call first-class functions and
    // evaluate into the global environment.
    std::shared_ptr<Env> globalEnv() { return global; }
    static Evaluator*& current() { thread_local Evaluator* t = nullptr; return t; }

private:
    std::shared_ptr<Env> global;

    Value evalProgram(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalBlock(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalIdent(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalInfix(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalPrefix(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalPostfix(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalCall(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalIf(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalWhile(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalLoop(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalForIn(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalObjLit(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalIndex(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalDot(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalSay(const ASTNode& node, std::shared_ptr<Env> env);

    LValueRef resolveLValue(const ASTNode& node, std::shared_ptr<Env> env,
                             Value& errOut);

    Value checkedAdd(int64_t a, int64_t b);
    Value checkedSub(int64_t a, int64_t b);
    Value checkedMul(int64_t a, int64_t b);
    Value checkedDiv(int64_t a, int64_t b);
    Value checkedMod(int64_t a, int64_t b);
};

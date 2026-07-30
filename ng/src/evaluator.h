#pragma once
#include "value.h"
#include "ast.h"
#include "env.h"
#include <memory>

class Evaluator {
public:
    explicit Evaluator(std::shared_ptr<Env> global)
        : global(std::move(global)) {}

    Value eval(const ASTNode& node, std::shared_ptr<Env> env);

private:
    std::shared_ptr<Env> global;

    Value evalProgram(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalBlock(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalIdent(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalInfix(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalPrefix(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalCall(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalIf(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalWhile(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalLoop(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalIndex(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalDot(const ASTNode& node, std::shared_ptr<Env> env);
    Value evalSay(const ASTNode& node, std::shared_ptr<Env> env);

    Value applyFunc(const Value& fn, const std::vector<Value>& args);
    Value checkedAdd(int64_t a, int64_t b);
    Value checkedSub(int64_t a, int64_t b);
    Value checkedMul(int64_t a, int64_t b);
    Value checkedDiv(int64_t a, int64_t b);
    Value checkedMod(int64_t a, int64_t b);
};

#pragma once
#include <memory>
#include <string>
#include "ast.h"
#include "value.h"

struct JITEngine {
    JITEngine();
    ~JITEngine();

    bool compile(const ASTNode& ast);
    bool isAvailable() const { return available; }
    int64_t executeCompiled(const std::string& fnName);

private:
    bool available = false;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

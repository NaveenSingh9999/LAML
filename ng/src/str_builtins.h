#pragma once
#include <memory>
struct Env;
void registerStrBuiltins(std::shared_ptr<Env> env);

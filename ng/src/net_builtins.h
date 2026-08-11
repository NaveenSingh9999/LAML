#pragma once
#include <memory>
struct Env;
void registerNetBuiltins(std::shared_ptr<Env> env);

#pragma once
#include <memory>
struct Env;
void registerSysBuiltins(std::shared_ptr<Env> env);
void sysInit(int argc, char** argv);

#pragma once
#include <memory>
struct Env;
void rtBridgeSetGlobal(std::shared_ptr<Env> env);
void registerRtBuiltins(std::shared_ptr<Env> env);

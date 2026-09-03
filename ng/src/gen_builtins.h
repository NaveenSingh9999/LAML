#pragma once
#include <memory>
struct Env;
// General-purpose stdlib: math, arrays, strings, objects, conversion, fs, assert.
void registerGenBuiltins(std::shared_ptr<Env> env);

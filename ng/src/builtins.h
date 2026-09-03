#pragma once
#include "value.h"
#include "env.h"
#include <mutex>

void registerBuiltins(std::shared_ptr<Env> env);
// Serializes say/print/evalSay across RT workers + closc threads.
std::mutex& lamlPrintMutex();

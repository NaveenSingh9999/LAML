#pragma once
#include <string>
#include "value.h"

// Dependency-free JSON. Limits: 1MB doc, 128 nesting depth.
// jsonParse: null->nil, true/false->bool, ints that fit -> int else float,
//   strings with escapes incl \uXXXX (BMP; surrogates paired), arrays->Array,
//   objects->Obj (duplicate keys: last wins). Errors as Value::Error, never throws.
// jsonStringify: cycle-guarded by depth cap (error beyond), binary strings raw,
//   floats via shortest round-trip, keys sorted? No — insertion order (Env unordered:
//   documented unordered).
struct RtJson {
    static Value parse(const std::string& doc);
    static Value stringify(const Value& v);
};

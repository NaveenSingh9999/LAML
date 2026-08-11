#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "value.h"

struct Env {
    std::unordered_map<std::string, Value> store;
    std::unordered_set<std::string> ro;   // read-only bindings (val)
    std::shared_ptr<Env> outer;

    explicit Env(std::shared_ptr<Env> outer = nullptr)
        : outer(std::move(outer)) {}

    Value* get(const std::string& name) {
        auto it = store.find(name);
        if (it != store.end()) return &it->second;
        if (outer) return outer->get(name);
        return nullptr;
    }

    bool isReadOnly(const std::string& name) const {
        const Env* target = this;
        while (target) {
            if (target->store.find(name) != target->store.end())
                return target->ro.count(name) > 0;
            target = target->outer.get();
        }
        return false;
    }

    // Declare a binding in THIS environment (shadows any outer binding).
    void declare(const std::string& name, Value val, bool readonly = false) {
        if (readonly) ro.insert(name);
        store[name] = std::move(val);
    }

    // Assign to an existing binding found up the chain; creates one here if absent.
    // Returns false when the target binding is read-only (val).
    bool set(const std::string& name, Value val) {
        Env* target = this;
        while (target && target->store.find(name) == target->store.end())
            target = target->outer.get();
        if (target) {
            if (target->ro.count(name)) return false;
            target->store[name] = std::move(val);
        } else {
            store[name] = std::move(val);
        }
        return true;
    }
};

#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cstdio>
#include "value.h"

struct Env {
    std::unordered_map<std::string, Value> store;
    std::unordered_set<std::string> ro;   // read-only bindings (val)
    std::shared_ptr<Env> outer;
    mutable std::mutex mtx;

    explicit Env(std::shared_ptr<Env> outer = nullptr)
        : outer(std::move(outer)) {}

    // Look up a binding up the chain. Returns a copy so the caller never
    // holds a pointer into a map another thread may rehash.
    std::optional<Value> get(const std::string& name) {
        Env* target = this;
        while (target) {
            std::lock_guard<std::mutex> lock(target->mtx);
            auto it = target->store.find(name);
            if (it != target->store.end()) return it->second;
            target = target->outer.get();
        }
        return std::nullopt;
    }

    bool isReadOnly(const std::string& name) const {
        const Env* target = this;
        while (target) {
            std::lock_guard<std::mutex> lock(target->mtx);
            if (target->store.find(name) != target->store.end())
                return target->ro.count(name) > 0;
            target = target->outer.get();
        }
        return false;
    }

    // Declare a binding in THIS environment (shadows any outer binding).
    void declare(const std::string& name, Value val, bool readonly = false) {
        std::lock_guard<std::mutex> lock(mtx);
        if (readonly) ro.insert(name);
        store[name] = std::move(val);
    }

    // Assign to an existing binding found up the chain; creates one here if absent.
    // Returns false when the target binding is read-only (val).
    bool set(const std::string& name, Value val) {
        Env* target = this;
        while (target) {
            std::lock_guard<std::mutex> lock(target->mtx);
            if (target->store.find(name) != target->store.end()) {
                if (target->ro.count(name)) return false;
                target->store[name] = std::move(val);
                return true;
            }
            target = target->outer.get();
        }
        std::lock_guard<std::mutex> lock(mtx);
        store[name] = std::move(val);
        return true;
    }
};

#pragma once
#include <memory>
#include <unordered_map>
#include "value.h"

struct Env {
    std::unordered_map<std::string, Value> store;
    std::shared_ptr<Env> outer;

    explicit Env(std::shared_ptr<Env> outer = nullptr)
        : outer(std::move(outer)) {}

    Value* get(const std::string& name) {
        auto it = store.find(name);
        if (it != store.end()) return &it->second;
        if (outer) return outer->get(name);
        return nullptr;
    }

    void set(const std::string& name, Value val) {
        Env* target = this;
        while (target && target->store.find(name) == target->store.end())
            target = target->outer.get();
        if (target) target->store[name] = std::move(val);
        else store[name] = std::move(val);
    }
};

#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <string>
#include <optional>
#include <unordered_map>
#include "value.h"
#include "ast.h"
#include "env.h"

struct Task {
    std::function<Value()> fn;
    std::vector<std::string> dependencies;
    std::string name;
    Value result;
    bool done = false;
    std::mutex mtx;
    std::condition_variable cv;
};

class Scheduler {
public:
    static Scheduler& instance() {
        static Scheduler sched;
        return sched;
    }

    void submit(std::shared_ptr<Task> task);
    Value runAndWait(std::shared_ptr<Task> task);
    void waitFor(const std::string& name);
    void markDone(const std::string& name, Value result);
    std::optional<Value> getResult(const std::string& name);

    void start();
    void stop();

    size_t workerCount() const { return workers.size(); }

private:
    std::vector<std::thread> workers;
    std::queue<std::shared_ptr<Task>> queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> running{false};
    std::unordered_map<std::string, Value> results;
    std::condition_variable resultsCv;
    std::mutex resultsMtx;

    void workerLoop();
};

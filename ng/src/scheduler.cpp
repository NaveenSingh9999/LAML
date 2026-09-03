#include "scheduler.h"
#include <iostream>

void Scheduler::start() {
    if (running) return;
    running = true;
    size_t count = std::thread::hardware_concurrency();
    if (count == 0) count = 4;
    for (size_t i = 0; i < count; i++) {
        workers.emplace_back(&Scheduler::workerLoop, this);
    }
}

void Scheduler::stop() {
    running = false;
    cv.notify_all();
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
}

void Scheduler::submit(std::shared_ptr<Task> task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(task));
    }
    cv.notify_one();
}

Value Scheduler::runAndWait(std::shared_ptr<Task> task) {
    submit(task);
    std::unique_lock<std::mutex> lock(task->mtx);
    task->cv.wait(lock, [&] { return task->done; });
    return task->result;
}

void Scheduler::markDone(const std::string& name, Value result) {
    std::lock_guard<std::mutex> lock(resultsMtx);
    results[name] = std::move(result);
    resultsCv.notify_all();
}

void Scheduler::waitFor(const std::string& name) {
    std::unique_lock<std::mutex> lock(resultsMtx);
    resultsCv.wait(lock, [&] { return results.find(name) != results.end(); });
}

std::optional<Value> Scheduler::getResult(const std::string& name) {
    std::lock_guard<std::mutex> lock(resultsMtx);
    auto it = results.find(name);
    if (it == results.end()) return std::nullopt;
    return it->second;
}

void Scheduler::workerLoop() {
    while (running) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::milliseconds(100), [&] {
                return !queue.empty() || !running;
            });
            if (!running) return;
            if (queue.empty()) continue;
            task = std::move(queue.front());
            queue.pop();
        }
        if (task) {
            task->result = task->fn();
            {
                std::lock_guard<std::mutex> lock(task->mtx);
                task->done = true;
            }
            task->cv.notify_all();
            markDone(task->name, task->result);
        }
    }
}

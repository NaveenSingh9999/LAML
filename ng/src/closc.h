#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

struct CloscTask {
    std::string name;
    int priority = 0;
    std::function<void()> fn;
    std::atomic<bool> running{false};
    std::atomic<bool> stop{false};
    std::thread thread;

    void start();
    void join();
    void requestStop() { stop = true; }
};

struct CloscManager {
    static CloscManager& instance() {
        static CloscManager mgr;
        return mgr;
    }

    int registerSection(const std::string& name, int priority, std::function<void()> fn);
    void stopAll();
    void waitAll();
    void killProcess();
    size_t liveCount();
    static bool isStopping() { return stopping_; }

private:
    std::vector<std::unique_ptr<CloscTask>> tasks;
    std::mutex mtx;
    inline static std::atomic<bool> stopping_{false};
    CloscManager() = default;
    ~CloscManager();
};

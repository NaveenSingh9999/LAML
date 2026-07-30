#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>

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

private:
    std::vector<std::unique_ptr<CloscTask>> tasks;
    CloscManager() = default;
    ~CloscManager();
};

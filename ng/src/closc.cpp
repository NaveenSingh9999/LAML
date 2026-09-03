#include "closc.h"
#include <iostream>
#include <cstdlib>

void CloscTask::start() {
    running = true;
    thread = std::thread([this]() {
        fn();
        running = false;
    });
}

void CloscTask::join() {
    if (thread.joinable()) thread.join();
}

CloscManager::~CloscManager() {}  // stopAll called explicitly

int CloscManager::registerSection(const std::string& name, int priority,
                                    std::function<void()> fn) {
    std::lock_guard<std::mutex> lock(mtx);
    auto task = std::make_unique<CloscTask>();
    task->name = name;
    task->priority = priority;
    task->fn = std::move(fn);
    int id = tasks.size();
    tasks.push_back(std::move(task));
    tasks.back()->start();
    return id;
}

size_t CloscManager::liveCount() {
    std::lock_guard<std::mutex> lock(mtx);
    size_t n = 0;
    for (auto& t : tasks) if (t && t->running) n++;
    return n;
}

void CloscManager::stopAll() {
    stopping_ = true;
    std::vector<CloscTask*> snap;
    { std::lock_guard<std::mutex> lock(mtx);
      for (auto& t : tasks) if (t) { t->requestStop(); snap.push_back(t.get()); } }
    for (auto* t : snap) t->join();
    std::lock_guard<std::mutex> lock(mtx);
    tasks.clear();
}

void CloscManager::waitAll() {
    // Snapshot threads under lock, join without it: a closc body may
    // itself register a new section (re-entrant), which must not block.
    std::vector<CloscTask*> snap;
    { std::lock_guard<std::mutex> lock(mtx);
      for (auto& t : tasks) if (t) snap.push_back(t.get()); }
    for (auto* t : snap) t->join();
}

void CloscManager::killProcess() {
    std::cerr << "\n[CLOSC] Killing process..." << std::endl;
    std::_Exit(137);
}

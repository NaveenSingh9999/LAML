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
    auto task = std::make_unique<CloscTask>();
    task->name = name;
    task->priority = priority;
    task->fn = std::move(fn);
    int id = tasks.size();
    tasks.push_back(std::move(task));
    tasks.back()->start();
    return id;
}

void CloscManager::stopAll() {
    for (auto& t : tasks) {
        if (t) {
            t->requestStop();
            t->join();
        }
    }
    tasks.clear();
}

void CloscManager::waitAll() {
    for (auto& t : tasks) {
        if (t && t->thread.joinable()) t->thread.join();
    }
}

void CloscManager::killProcess() {
    std::cerr << "\n[CLOSC] Killing process..." << std::endl;
    std::_Exit(137);
}

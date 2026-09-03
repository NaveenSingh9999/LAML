#include "rt_timer.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <unordered_map>
#include <atomic>
#include <chrono>

namespace {
struct Entry {
    int64_t id;
    int64_t intervalMs; // 0 = one-shot
    int64_t dueMs;
    std::function<void()> cb;
};
std::mutex m;
std::condition_variable cv;
std::multimap<std::pair<int64_t,int64_t>, std::function<void()>> q; // (due,id) -> cb
std::unordered_map<int64_t, int64_t> idDue; // id -> current due
std::unordered_map<int64_t, int64_t> idIv;  // id -> interval (0 = one-shot)
std::atomic<int64_t> nextId{1};
std::atomic<bool> stopFlag{false};
std::thread th;
bool started = false;

int64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
void loop() {
    while (!stopFlag) {
        std::vector<std::function<void()>> due;
        { std::unique_lock<std::mutex> l(m);
          cv.wait_for(l, std::chrono::milliseconds(50), []{ return stopFlag.load() || !q.empty(); });
          if (stopFlag) return;
          int64_t now = nowMs();
          while (!q.empty() && q.begin()->first.first <= now) {
              auto it = q.begin();
              due.push_back(std::move(it->second));
              int64_t id = it->first.second;
              auto jt = idIv.find(id);
              int64_t iv = (jt == idIv.end()) ? 0 : jt->second;
              q.erase(it);
              if (iv > 0 && idIv.count(id)) {
                  int64_t nd = now + iv;
                  q.emplace(std::make_pair(nd, id), due.back());
                  idDue[id] = nd;
              } else {
                  idIv.erase(id);
                  idDue.erase(id);
              }
          } }
        for (auto& cb : due) {
            if (stopFlag) return;
            try { cb(); } catch (...) {}
        }
    }
}
}

void TimerWheel::start() {
    std::lock_guard<std::mutex> l(m);
    if (started) return;
    started = true;
    stopFlag = false;
    th = std::thread(loop);
}

void TimerWheel::stop() {
    stopFlag = true;
    cv.notify_all();
    if (th.joinable()) th.join();
    std::lock_guard<std::mutex> l(m);
    q.clear(); idDue.clear(); idIv.clear(); started = false;
}

int64_t TimerWheel::add(int64_t delayMs, int64_t intervalMs, std::function<void()> cb) {
    if (delayMs < 0) delayMs = 0;
    if (delayMs > 24*3600*1000) delayMs = 24*3600*1000;
    int64_t id = nextId++;
    int64_t due = nowMs() + delayMs;
    std::lock_guard<std::mutex> l(m);
    q.emplace(std::make_pair(due, id), std::move(cb));
    idDue[id] = due;
    idIv[id] = intervalMs;
    cv.notify_one();
    return id;
}

void TimerWheel::cancel(int64_t id) {
    std::lock_guard<std::mutex> l(m);
    auto it = idDue.find(id);
    if (it == idDue.end()) return;
    auto key = std::make_pair(it->second, id);
    auto range = q.equal_range(key);
    for (auto jt = range.first; jt != range.second; ++jt) {
        if (jt->first.second == id) { q.erase(jt); break; }
    }
    idDue.erase(it);
    idIv.erase(id);
}

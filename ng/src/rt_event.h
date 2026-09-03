#pragma once
#include <string>
#include <cstdint>
#include <deque>
#include <mutex>
#include <condition_variable>

enum class EvType { Connect, Message, Request, Datagram, Close, Timeout, Drain };

struct Event {
    int fd = -1;
    EvType type = EvType::Request;
    std::string topic;
    std::string payload;
    uint64_t tRecvNs = 0;
};

struct Action {
    int fd = -1;
    std::string room;
    std::string payload;
    bool broadcast = false;
    bool closeAfter = false;
};

// M1: mutex+condvar queue (correct first, lock-free in M4 if profile demands).
// Capacity 65536. Push of Request may drop-newest (return false => caller 503s).
// Close/Timeout/Drain never dropped (caller retries).
class EventQueue {
public:
    explicit EventQueue(size_t cap = 65536) : cap_(cap) {}
    bool push(Event&& e, bool droppable) {
        std::lock_guard<std::mutex> l(m_);
        if (q_.size() >= cap_) {
            if (droppable) { dropped_++; return false; }
            // non-droppable: evict oldest droppable if any, else fail
            return false;
        }
        q_.push_back(std::move(e));
        cv_.notify_one();
        return true;
    }
    bool pop(Event& out, int timeoutMs) {
        std::unique_lock<std::mutex> l(m_);
        if (timeoutMs < 0) cv_.wait(l, [&]{ return !q_.empty() || stopped_; });
        else cv_.wait_for(l, std::chrono::milliseconds(timeoutMs), [&]{ return !q_.empty() || stopped_; });
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop_front();
        return true;
    }
    void stop() { std::lock_guard<std::mutex> l(m_); stopped_ = true; cv_.notify_all(); }
    uint64_t dropped() const { return dropped_; }
private:
    std::deque<Event> q_;
    mutable std::mutex m_;
    std::condition_variable cv_;
    size_t cap_;
    bool stopped_ = false;
    uint64_t dropped_ = 0;
};

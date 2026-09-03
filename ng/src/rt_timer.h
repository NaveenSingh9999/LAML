#pragma once
#include <functional>
#include <cstdint>
#include <string>

// Single-wheel timer service. One thread ticks every 50ms, fires due
// callbacks on a small pool (callbacks must be fast; LAML timer handlers
// run via bridge which reuses RT workers — never on the tick thread).
// Jitter: caller spreads phases; wheel slot scan is amortized O(1).
class TimerWheel {
public:
    static TimerWheel& instance() { static TimerWheel w; return w; }
    void start();
    void stop();
    // Returns id. intervalMs==0 => one-shot timeout, else repeats.
    int64_t add(int64_t delayMs, int64_t intervalMs, std::function<void()> cb);
    void cancel(int64_t id);
private:
    TimerWheel() = default;
};

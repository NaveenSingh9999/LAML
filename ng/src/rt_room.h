#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <array>
#include <functional>

// In-process pub/sub: room -> fds. 16 shards, each own mutex.
// Mutated from loop thread + worker threads, so sharded locking matters.
// Presence is free: members().
class RoomBus {
public:
    static RoomBus& instance() { static RoomBus b; return b; }

    void join(int fd, const std::string& room);
    void leave(int fd, const std::string& room);
    void leaveAll(int fd);
    std::vector<int> members(const std::string& room);
    size_t roomCount();
    // Fan-out: calls sendOne(fd) for each member (except exceptFd).
    // sendOne must be non-blocking (queue to loop). Returns delivered count.
    int broadcast(const std::string& room, std::function<void(int)> sendOne, int exceptFd = -1);
private:
    static constexpr int kShards = 16;
    struct Shard {
        std::mutex m;
        std::unordered_map<std::string, std::unordered_set<int>> rooms;
        std::unordered_map<int, std::unordered_set<std::string>> byFd;
    };
    std::array<Shard, kShards> shards_;
    Shard& shardFor(const std::string& room);
};

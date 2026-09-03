#include "rt_room.h"
#include <functional>

RoomBus::Shard& RoomBus::shardFor(const std::string& room) {
    return shards_[std::hash<std::string>{}(room) % kShards];
}

void RoomBus::join(int fd, const std::string& room) {
    if (room.empty() || room.size() > 256) return;
    Shard& s = shardFor(room);
    std::lock_guard<std::mutex> l(s.m);
    s.rooms[room].insert(fd);
    s.byFd[fd].insert(room);
}

void RoomBus::leave(int fd, const std::string& room) {
    Shard& s = shardFor(room);
    std::lock_guard<std::mutex> l(s.m);
    auto it = s.rooms.find(room);
    if (it != s.rooms.end()) {
        it->second.erase(fd);
        if (it->second.empty()) s.rooms.erase(it);
    }
    auto jt = s.byFd.find(fd);
    if (jt != s.byFd.end()) {
        jt->second.erase(room);
        if (jt->second.empty()) s.byFd.erase(jt);
    }
}

void RoomBus::leaveAll(int fd) {
    for (auto& s : shards_) {
        std::lock_guard<std::mutex> l(s.m);
        auto jt = s.byFd.find(fd);
        if (jt == s.byFd.end()) continue;
        for (auto& room : jt->second) {
            auto it = s.rooms.find(room);
            if (it != s.rooms.end()) {
                it->second.erase(fd);
                if (it->second.empty()) s.rooms.erase(it);
            }
        }
        s.byFd.erase(jt);
    }
}

std::vector<int> RoomBus::members(const std::string& room) {
    Shard& s = shardFor(room);
    std::lock_guard<std::mutex> l(s.m);
    auto it = s.rooms.find(room);
    if (it == s.rooms.end()) return {};
    return std::vector<int>(it->second.begin(), it->second.end());
}

size_t RoomBus::roomCount() {
    size_t n = 0;
    for (auto& s : shards_) {
        std::lock_guard<std::mutex> l(s.m);
        n += s.rooms.size();
    }
    return n;
}

int RoomBus::broadcast(const std::string& room, std::function<void(int)> sendOne, int exceptFd) {
    // Snapshot under lock, send outside lock (sendOne queues, non-blocking).
    std::vector<int> fds;
    { Shard& s = shardFor(room);
      std::lock_guard<std::mutex> l(s.m);
      auto it = s.rooms.find(room);
      if (it == s.rooms.end()) return 0;
      fds.assign(it->second.begin(), it->second.end()); }
    int n = 0;
    for (int fd : fds) {
        if (fd == exceptFd) continue;
        sendOne(fd);
        n++;
    }
    return n;
}

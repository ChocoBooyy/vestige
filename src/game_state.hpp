#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "world/log_entry.hpp"

namespace vestige {

class GameState {
public:
    std::string currentRoom;

    bool addItem(const std::string& id, int maxItems);
    bool hasItem(const std::string& id) const;
    void removeItem(const std::string& id);
    const std::vector<std::string>& inventory() const { return inventory_; }

    void markItemTaken(const std::string& id) { takenItems_.insert(id); }
    bool itemTaken(const std::string& id) const { return takenItems_.count(id) != 0; }

    void visitRoom(const std::string& id) { visited_.insert(id); }
    bool visited(const std::string& id) const { return visited_.count(id) != 0; }
    const std::set<std::string>& visitedRooms() const { return visited_; }

    void discoverLog(const std::string& id, LogState state);
    bool logDiscovered(const std::string& id) const { return logStates_.count(id) != 0; }
    LogState logState(const std::string& id) const;
    void setLogState(const std::string& id, LogState state) { logStates_[id] = state; }
    const std::map<std::string, LogState>& discoveredLogs() const { return logStates_; }

    void markLogRead(const std::string& id) { logsRead_.insert(id); }
    bool logRead(const std::string& id) const { return logsRead_.count(id) != 0; }
    const std::set<std::string>& logsRead() const { return logsRead_; }

    void setLogVersion(const std::string& id, LogVersion version) { logVersions_[id] = version; }
    LogVersion logVersion(const std::string& id) const;
    const std::map<std::string, LogVersion>& logVersions() const { return logVersions_; }

    void solvePuzzle(const std::string& id) { solved_.insert(id); }
    bool puzzleSolved(const std::string& id) const { return solved_.count(id) != 0; }
    const std::set<std::string>& solvedPuzzles() const { return solved_; }

    void setFlag(const std::string& id) { flags_.insert(id); }
    bool flag(const std::string& id) const { return flags_.count(id) != 0; }
    const std::set<std::string>& flags() const { return flags_; }

    const std::set<std::string>& takenItems() const { return takenItems_; }

private:
    std::vector<std::string> inventory_;
    std::set<std::string> takenItems_;
    std::set<std::string> visited_;
    std::map<std::string, LogState> logStates_;
    std::set<std::string> logsRead_;
    std::map<std::string, LogVersion> logVersions_;
    std::set<std::string> solved_;
    std::set<std::string> flags_;
};

}

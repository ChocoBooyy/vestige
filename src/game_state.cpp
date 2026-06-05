#include "game_state.hpp"

#include <algorithm>

namespace vestige {

bool GameState::addItem(const std::string& id, int maxItems) {
    if (static_cast<int>(inventory_.size()) >= maxItems) {
        return false;
    }
    if (hasItem(id)) {
        return true;
    }
    inventory_.push_back(id);
    return true;
}

bool GameState::hasItem(const std::string& id) const {
    return std::find(inventory_.begin(), inventory_.end(), id) != inventory_.end();
}

void GameState::removeItem(const std::string& id) {
    inventory_.erase(std::remove(inventory_.begin(), inventory_.end(), id), inventory_.end());
}

void GameState::discoverLog(const std::string& id, LogState state) {
    if (logStates_.find(id) == logStates_.end()) {
        logStates_[id] = state;
    }
}

LogState GameState::logState(const std::string& id) const {
    const auto it = logStates_.find(id);
    return it == logStates_.end() ? LogState::Sealed : it->second;
}

LogVersion GameState::logVersion(const std::string& id) const {
    const auto it = logVersions_.find(id);
    return it == logVersions_.end() ? LogVersion::Unchosen : it->second;
}

}

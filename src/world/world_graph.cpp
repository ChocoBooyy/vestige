#include "world/world_graph.hpp"

#include "util/string_util.hpp"

namespace vestige {

void WorldGraph::addRoom(Room room) {
    const std::string id = room.id;
    if (rooms_.find(id) == rooms_.end()) {
        roomOrder_.push_back(id);
    }
    rooms_[id] = std::move(room);
}

void WorldGraph::addItem(Item item) {
    const std::string id = item.id;
    if (items_.find(id) == items_.end()) {
        itemOrder_.push_back(id);
    }
    items_[id] = std::move(item);
}

void WorldGraph::addLog(LogEntry log) {
    const std::string id = log.id;
    if (logs_.find(id) == logs_.end()) {
        logOrder_.push_back(id);
    }
    logs_[id] = std::move(log);
}

void WorldGraph::setStartRoom(const std::string& id) {
    startRoom_ = id;
}

const Room* WorldGraph::room(const std::string& id) const {
    const auto it = rooms_.find(id);
    return it == rooms_.end() ? nullptr : &it->second;
}

const Item* WorldGraph::item(const std::string& id) const {
    const auto it = items_.find(id);
    return it == items_.end() ? nullptr : &it->second;
}

const LogEntry* WorldGraph::log(const std::string& id) const {
    const auto it = logs_.find(id);
    return it == logs_.end() ? nullptr : &it->second;
}

std::vector<std::string> WorldGraph::itemsDefinedInRoom(const std::string& roomId) const {
    std::vector<std::string> result;
    for (const std::string& id : itemOrder_) {
        const Item& candidate = items_.at(id);
        if (candidate.startRoom == roomId) {
            result.push_back(id);
        }
    }
    return result;
}

const Exit* WorldGraph::findExit(const std::string& roomId, const std::string& direction) const {
    const Room* current = room(roomId);
    if (current == nullptr) {
        return nullptr;
    }
    for (const Exit& exit : current->exits) {
        if (exit.direction == direction) {
            return &exit;
        }
    }
    for (const Exit& exit : current->exits) {
        if (strutil::partialMatch(exit.direction, direction)) {
            return &exit;
        }
    }
    return nullptr;
}

}

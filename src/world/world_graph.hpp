#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "world/item.hpp"
#include "world/log_entry.hpp"
#include "world/room.hpp"

namespace vestige {

class WorldGraph {
public:
    void addRoom(Room room);
    void addItem(Item item);
    void addLog(LogEntry log);
    void setStartRoom(const std::string& id);

    const Room* room(const std::string& id) const;
    const Item* item(const std::string& id) const;
    const LogEntry* log(const std::string& id) const;

    const std::string& startRoom() const { return startRoom_; }
    const std::vector<std::string>& roomOrder() const { return roomOrder_; }
    const std::vector<std::string>& logOrder() const { return logOrder_; }
    const std::vector<std::string>& itemOrder() const { return itemOrder_; }

    std::vector<std::string> itemsDefinedInRoom(const std::string& roomId) const;
    const Exit* findExit(const std::string& roomId, const std::string& direction) const;

private:
    std::unordered_map<std::string, Room> rooms_;
    std::unordered_map<std::string, Item> items_;
    std::unordered_map<std::string, LogEntry> logs_;
    std::vector<std::string> roomOrder_;
    std::vector<std::string> logOrder_;
    std::vector<std::string> itemOrder_;
    std::string startRoom_;
};

}

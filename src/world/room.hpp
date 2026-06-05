#pragma once

#include <string>
#include <vector>

#include "world/exit.hpp"

namespace vestige {

struct Room {
    std::string id;
    std::string name;
    std::string sector;
    std::string description;
    std::vector<Exit> exits;
    std::vector<std::string> logIds;
    std::vector<std::string> puzzleIds;
    std::string aeonTagOnEnter;
    bool terminus = false;
};

}

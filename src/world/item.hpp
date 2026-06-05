#pragma once

#include <string>

namespace vestige {

enum class ItemEffect {
    None,
    SetFlag,
    AeonLine,
    RevealLog
};

struct Item {
    std::string id;
    std::string name;
    std::string description;
    ItemEffect effect = ItemEffect::None;
    std::string effectTarget;
    std::string useMessage;
    std::string startRoom;
};

}

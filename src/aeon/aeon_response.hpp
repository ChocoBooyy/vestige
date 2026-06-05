#pragma once

#include <string>

#include "terminal.hpp"

namespace vestige {

enum class AeonTrigger {
    EnterRoom,
    LogRead,
    PuzzleSolved,
    VersionChosen,
    Awakening
};

struct AeonResponse {
    std::string id;
    AeonTrigger trigger = AeonTrigger::EnterRoom;
    std::string triggerValue;
    int minTrust = -1000;
    int maxTrust = 1000;
    int minAwareness = -1000;
    int maxAwareness = 1000;
    std::string line;
    Voice voice = Voice::Aeon;
    bool once = true;
};

}

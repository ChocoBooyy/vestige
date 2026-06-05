#pragma once

#include <string>

namespace vestige {

enum class ConditionType {
    None,
    PuzzleSolved,
    LogRead,
    ItemHeld,
    TrustAbove,
    TrustBelow,
    AwarenessAbove,
    AwarenessBelow,
    FlagSet
};

struct Condition {
    ConditionType type = ConditionType::None;
    std::string targetId;
    int threshold = 0;
};

}

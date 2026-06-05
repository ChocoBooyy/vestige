#pragma once

#include <string>

#include "world/condition.hpp"

namespace vestige {

struct Exit {
    std::string direction;
    std::string targetId;
    Condition condition;
    std::string lockedMessage;
};

}

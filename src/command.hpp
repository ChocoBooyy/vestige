#pragma once

#include <string>

namespace vestige {

enum class Verb {
    Go,
    Examine,
    Take,
    Use,
    Read,
    Inventory,
    Map,
    Codex,
    Status,
    Help,
    Save,
    Load,
    Quit,
    Unknown,
    Empty
};

struct Command {
    Verb verb = Verb::Empty;
    std::string noun;
    std::string target;
    std::string raw;
};

}

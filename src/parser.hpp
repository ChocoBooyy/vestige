#pragma once

#include <string>

#include "command.hpp"

namespace vestige {

class Parser {
public:
    Command parse(const std::string& rawInput) const;

private:
    Verb resolveVerb(const std::string& token) const;
};

}

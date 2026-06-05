#pragma once

#include <string>

#include "puzzle/puzzle.hpp"

namespace vestige {

class LogicPuzzle final : public Puzzle {
public:
    LogicPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                std::string brokenExpression, std::string answer);

    void present(Terminal& terminal) const override;
    bool attempt(const std::string& input) const override;

private:
    static std::string normalize(const std::string& expression);

    std::string brokenExpression_;
    std::string answer_;
};

}

#pragma once

#include <string>

#include "puzzle/puzzle.hpp"

namespace vestige {

class RedactionPuzzle final : public Puzzle {
public:
    RedactionPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                    std::string redactedText, std::string contextText, std::string answer);

    void present(Terminal& terminal) const override;
    bool attempt(const std::string& input) const override;

private:
    std::string redactedText_;
    std::string contextText_;
    std::string answer_;
};

}

#pragma once

#include <string>
#include <vector>

#include "puzzle/puzzle.hpp"

namespace vestige {

class SequencePuzzle final : public Puzzle {
public:
    SequencePuzzle(std::string id, std::string linkedLogId, std::string prompt,
                   std::vector<std::string> steps, std::vector<int> correctOrder);

    void present(Terminal& terminal) const override;
    bool attempt(const std::string& input) const override;

private:
    std::vector<std::string> steps_;
    std::vector<int> correctOrder_;
};

}

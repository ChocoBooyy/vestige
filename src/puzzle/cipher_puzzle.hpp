#pragma once

#include <string>

#include "puzzle/puzzle.hpp"

namespace vestige {

class CipherPuzzle final : public Puzzle {
public:
    CipherPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                 std::string cipherText, std::string keyHint, std::string answer);

    void present(Terminal& terminal) const override;
    bool attempt(const std::string& input) const override;

private:
    std::string cipherText_;
    std::string keyHint_;
    std::string answer_;
};

}

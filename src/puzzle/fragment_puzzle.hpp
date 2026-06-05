#pragma once

#include <string>
#include <vector>

#include "puzzle/puzzle.hpp"

namespace vestige {

struct Fragment {
    std::string id;
    std::string text;
};

class FragmentPuzzle final : public Puzzle {
public:
    FragmentPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                   std::vector<Fragment> fragments, std::string oddId);

    void present(Terminal& terminal) const override;
    bool attempt(const std::string& input) const override;

private:
    std::vector<Fragment> fragments_;
    std::string oddId_;
};

}

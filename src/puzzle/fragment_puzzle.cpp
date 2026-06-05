#include "puzzle/fragment_puzzle.hpp"

#include "util/string_util.hpp"

namespace vestige {

FragmentPuzzle::FragmentPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                               std::vector<Fragment> fragments, std::string oddId)
    : Puzzle(std::move(id), PuzzleType::Fragment, std::move(linkedLogId), std::move(prompt)),
      fragments_(std::move(fragments)),
      oddId_(std::move(oddId)) {}

void FragmentPuzzle::present(Terminal& terminal) const {
    terminal.say(Voice::System, prompt_);
    for (const Fragment& fragment : fragments_) {
        terminal.say(Voice::Log, "[" + fragment.id + "] " + fragment.text);
    }
    terminal.say(Voice::System, "Type the id of the fragment that does not belong.");
}

bool FragmentPuzzle::attempt(const std::string& input) const {
    const std::string cleaned = strutil::toLower(strutil::trim(input));
    return cleaned == strutil::toLower(oddId_);
}

}

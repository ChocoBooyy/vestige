#include "puzzle/sequence_puzzle.hpp"

#include <string>

#include "util/string_util.hpp"

namespace vestige {

SequencePuzzle::SequencePuzzle(std::string id, std::string linkedLogId, std::string prompt,
                               std::vector<std::string> steps, std::vector<int> correctOrder)
    : Puzzle(std::move(id), PuzzleType::Sequence, std::move(linkedLogId), std::move(prompt)),
      steps_(std::move(steps)),
      correctOrder_(std::move(correctOrder)) {}

void SequencePuzzle::present(Terminal& terminal) const {
    terminal.say(Voice::System, prompt_);
    for (std::size_t i = 0; i < steps_.size(); ++i) {
        terminal.say(Voice::Log, std::to_string(i + 1) + ") " + steps_[i]);
    }
    terminal.say(Voice::System, "Type the correct order as space-separated step numbers.");
}

bool SequencePuzzle::attempt(const std::string& input) const {
    const std::vector<std::string> tokens = strutil::words(input);
    if (tokens.size() != correctOrder_.size()) {
        return false;
    }
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        int value = 0;
        try {
            value = std::stoi(tokens[i]);
        } catch (const std::exception&) {
            return false;
        }
        if (value != correctOrder_[i]) {
            return false;
        }
    }
    return true;
}

}

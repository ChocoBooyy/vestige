#include "puzzle/logic_puzzle.hpp"

#include <cctype>

#include "util/string_util.hpp"

namespace vestige {

LogicPuzzle::LogicPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                         std::string brokenExpression, std::string answer)
    : Puzzle(std::move(id), PuzzleType::Logic, std::move(linkedLogId), std::move(prompt)),
      brokenExpression_(std::move(brokenExpression)),
      answer_(std::move(answer)) {}

std::string LogicPuzzle::normalize(const std::string& expression) {
    std::string out;
    for (char c : expression) {
        if (std::isspace(static_cast<unsigned char>(c)) != 0) {
            continue;
        }
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

void LogicPuzzle::present(Terminal& terminal) const {
    terminal.say(Voice::System, prompt_);
    terminal.say(Voice::Error, "FAULTED EXPRESSION: " + brokenExpression_);
    terminal.say(Voice::System, "Type the corrected expression.");
}

bool LogicPuzzle::attempt(const std::string& input) const {
    return normalize(input) == normalize(answer_);
}

}

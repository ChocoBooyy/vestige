#include "puzzle/redaction_puzzle.hpp"

#include "util/string_util.hpp"

namespace vestige {

RedactionPuzzle::RedactionPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                                 std::string redactedText, std::string contextText, std::string answer)
    : Puzzle(std::move(id), PuzzleType::Redaction, std::move(linkedLogId), std::move(prompt)),
      redactedText_(std::move(redactedText)),
      contextText_(std::move(contextText)),
      answer_(std::move(answer)) {}

void RedactionPuzzle::present(Terminal& terminal) const {
    terminal.say(Voice::System, prompt_);
    terminal.say(Voice::Rewritten, redactedText_);
    terminal.say(Voice::Log, "CROSS REFERENCE: " + contextText_);
    terminal.say(Voice::System, "Type the single word that fills the marked redaction.");
}

bool RedactionPuzzle::attempt(const std::string& input) const {
    return strutil::toLower(strutil::trim(input)) == strutil::toLower(strutil::trim(answer_));
}

}

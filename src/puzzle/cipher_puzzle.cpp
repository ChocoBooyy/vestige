#include "puzzle/cipher_puzzle.hpp"

#include "util/string_util.hpp"

namespace vestige {

CipherPuzzle::CipherPuzzle(std::string id, std::string linkedLogId, std::string prompt,
                           std::string cipherText, std::string keyHint, std::string answer)
    : Puzzle(std::move(id), PuzzleType::Cipher, std::move(linkedLogId), std::move(prompt)),
      cipherText_(std::move(cipherText)),
      keyHint_(std::move(keyHint)),
      answer_(std::move(answer)) {}

void CipherPuzzle::present(Terminal& terminal) const {
    terminal.say(Voice::System, prompt_);
    terminal.say(Voice::Error, "ENCODED: " + cipherText_);
    terminal.say(Voice::Log, "KEY FRAGMENT: " + keyHint_);
    terminal.say(Voice::System, "Type the decoded text.");
}

bool CipherPuzzle::attempt(const std::string& input) const {
    return strutil::toLower(strutil::collapseWhitespace(input)) ==
           strutil::toLower(strutil::collapseWhitespace(answer_));
}

}

#include "puzzle/puzzle.hpp"

namespace vestige {

Puzzle::Puzzle(std::string id, PuzzleType type, std::string linkedLogId, std::string prompt)
    : id_(std::move(id)),
      type_(type),
      linkedLogId_(std::move(linkedLogId)),
      prompt_(std::move(prompt)) {}

}

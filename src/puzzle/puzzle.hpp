#pragma once

#include <string>

#include "terminal.hpp"

namespace vestige {

enum class PuzzleType {
    Cipher,
    Sequence,
    Logic,
    Fragment,
    Redaction
};

class Puzzle {
public:
    Puzzle(std::string id, PuzzleType type, std::string linkedLogId, std::string prompt);
    virtual ~Puzzle() = default;

    Puzzle(const Puzzle&) = delete;
    Puzzle& operator=(const Puzzle&) = delete;

    const std::string& id() const { return id_; }
    PuzzleType type() const { return type_; }
    const std::string& linkedLogId() const { return linkedLogId_; }
    const std::string& prompt() const { return prompt_; }

    const std::string& requiredItem() const { return requiredItem_; }
    void setRequiredItem(std::string itemId) { requiredItem_ = std::move(itemId); }

    virtual void present(Terminal& terminal) const = 0;
    virtual bool attempt(const std::string& input) const = 0;

protected:
    std::string id_;
    PuzzleType type_;
    std::string linkedLogId_;
    std::string prompt_;
    std::string requiredItem_;
};

}

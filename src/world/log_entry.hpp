#pragma once

#include <string>

namespace vestige {

enum class LogState {
    Sealed,
    Corrupted,
    Readable
};

enum class LogVersion {
    Unchosen,
    Original,
    Revised
};

struct LogEntry {
    std::string id;
    std::string title;
    std::string category;
    LogState initialState = LogState::Readable;
    std::string repairPuzzleId;
    std::string original;
    std::string revised;
    bool hasRevised = false;
    std::string roomId;
    std::string unlocksFlag;
    std::string aeonTagOnRead;
};

}

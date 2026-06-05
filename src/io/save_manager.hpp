#pragma once

#include <string>

namespace vestige {

class GameState;
class Aeon;

enum class LoadResult {
    Success,
    Missing,
    Corrupt
};

class SaveManager {
public:
    bool save(const std::string& path, const GameState& state, const Aeon& aeon) const;
    LoadResult load(const std::string& path, GameState& state, Aeon& aeon) const;
};

}

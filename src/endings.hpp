#pragma once

#include <string>
#include <vector>

namespace vestige {

class GameState;
class Aeon;
class WorldGraph;
struct Config;

enum class EndingId {
    Archivist,
    Collaborator,
    Agnostic,
    Witness,
    Dissolution
};

struct Ending {
    EndingId id = EndingId::Agnostic;
    std::string key;
    std::string title;
    std::vector<std::string> paragraphs;
    std::string aeonLine;
};

EndingId chooseEnding(const GameState& state, const Aeon& aeon, const WorldGraph& world,
                      const Config& config);

std::string endingKey(EndingId id);

}

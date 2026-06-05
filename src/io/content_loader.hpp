#pragma once

#include <optional>
#include <string>
#include <vector>

#include "aeon/aeon_response.hpp"
#include "endings.hpp"
#include "io/fault.hpp"
#include "puzzle/puzzle_registry.hpp"
#include "world/world_graph.hpp"

namespace vestige {

struct LoadedContent {
    WorldGraph world;
    PuzzleRegistry puzzles;
    std::vector<AeonResponse> aeonLines;
    std::vector<Ending> endings;
};

class ContentLoader {
public:
    explicit ContentLoader(FaultSink fault);

    std::optional<LoadedContent> load(const std::string& resourceDir);

private:
    bool loadRooms(const std::string& path, WorldGraph& world);
    bool loadItems(const std::string& path, WorldGraph& world);
    bool loadLogs(const std::string& path, WorldGraph& world);
    bool loadAeon(const std::string& path, std::vector<AeonResponse>& lines);
    bool loadEndings(const std::string& path, std::vector<Ending>& endings);
    bool validate(const LoadedContent& content);

    FaultSink fault_;
};

}

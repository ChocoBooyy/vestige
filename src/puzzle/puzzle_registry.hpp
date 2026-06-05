#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "io/fault.hpp"
#include "puzzle/puzzle.hpp"

namespace vestige {

class PuzzleRegistry {
public:
    bool loadFromJson(const nlohmann::json& root, const FaultSink& fault);

    const Puzzle* get(const std::string& id) const;
    const std::vector<std::string>& order() const { return order_; }

private:
    std::unordered_map<std::string, std::unique_ptr<Puzzle>> puzzles_;
    std::vector<std::string> order_;
};

}

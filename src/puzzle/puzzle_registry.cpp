#include "puzzle/puzzle_registry.hpp"

#include <nlohmann/json.hpp>

#include "puzzle/cipher_puzzle.hpp"
#include "puzzle/fragment_puzzle.hpp"
#include "puzzle/logic_puzzle.hpp"
#include "puzzle/redaction_puzzle.hpp"
#include "puzzle/sequence_puzzle.hpp"

namespace vestige {

namespace {

std::unique_ptr<Puzzle> buildOne(const nlohmann::json& node) {
    const std::string id = node.at("id").get<std::string>();
    const std::string type = node.at("type").get<std::string>();
    const std::string prompt = node.value("prompt", std::string{});
    const std::string linkedLog = node.value("linkedLog", std::string{});

    if (type == "cipher") {
        return std::make_unique<CipherPuzzle>(
            id, linkedLog, prompt,
            node.at("cipherText").get<std::string>(),
            node.at("keyHint").get<std::string>(),
            node.at("answer").get<std::string>());
    }
    if (type == "sequence") {
        std::vector<std::string> steps = node.at("steps").get<std::vector<std::string>>();
        std::vector<int> order = node.at("correctOrder").get<std::vector<int>>();
        return std::make_unique<SequencePuzzle>(id, linkedLog, prompt, std::move(steps), std::move(order));
    }
    if (type == "logic") {
        return std::make_unique<LogicPuzzle>(
            id, linkedLog, prompt,
            node.at("brokenExpression").get<std::string>(),
            node.at("answer").get<std::string>());
    }
    if (type == "fragment") {
        std::vector<Fragment> fragments;
        for (const auto& fragmentNode : node.at("fragments")) {
            Fragment fragment;
            fragment.id = fragmentNode.at("id").get<std::string>();
            fragment.text = fragmentNode.at("text").get<std::string>();
            fragments.push_back(std::move(fragment));
        }
        return std::make_unique<FragmentPuzzle>(id, linkedLog, prompt, std::move(fragments),
                                                node.at("oddId").get<std::string>());
    }
    if (type == "redaction") {
        return std::make_unique<RedactionPuzzle>(
            id, linkedLog, prompt,
            node.at("redactedText").get<std::string>(),
            node.at("contextText").get<std::string>(),
            node.at("answer").get<std::string>());
    }
    return nullptr;
}

}

bool PuzzleRegistry::loadFromJson(const nlohmann::json& root, const FaultSink& fault) {
    if (!root.is_array()) {
        fault("puzzles.json root must be an array");
        return false;
    }
    for (const auto& node : root) {
        std::unique_ptr<Puzzle> puzzle;
        try {
            puzzle = buildOne(node);
        } catch (const std::exception& error) {
            fault(std::string{"puzzle parse error: "} + error.what());
            return false;
        }
        if (puzzle == nullptr) {
            fault("unknown puzzle type encountered");
            return false;
        }
        puzzle->setRequiredItem(node.value("requiresItem", std::string{}));
        const std::string id = puzzle->id();
        order_.push_back(id);
        puzzles_[id] = std::move(puzzle);
    }
    return true;
}

const Puzzle* PuzzleRegistry::get(const std::string& id) const {
    const auto it = puzzles_.find(id);
    return it == puzzles_.end() ? nullptr : it->second.get();
}

}

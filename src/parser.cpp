#include "parser.hpp"

#include <unordered_map>
#include <unordered_set>

#include "util/string_util.hpp"

namespace vestige {

namespace {

const std::unordered_set<std::string>& directionTokens() {
    static const std::unordered_set<std::string> tokens = {
        "north", "south", "east", "west", "up", "down", "in", "out",
        "n", "s", "e", "w", "u", "d"
    };
    return tokens;
}

std::string canonicalDirection(const std::string& token) {
    static const std::unordered_map<std::string, std::string> map = {
        {"n", "north"}, {"s", "south"}, {"e", "east"}, {"w", "west"},
        {"u", "up"}, {"d", "down"}
    };
    const auto it = map.find(token);
    return it == map.end() ? token : it->second;
}

const std::unordered_map<std::string, Verb>& verbTable() {
    static const std::unordered_map<std::string, Verb> table = {
        {"go", Verb::Go}, {"move", Verb::Go}, {"walk", Verb::Go}, {"head", Verb::Go},
        {"examine", Verb::Examine}, {"look", Verb::Examine}, {"inspect", Verb::Examine},
        {"x", Verb::Examine}, {"check", Verb::Examine},
        {"take", Verb::Take}, {"get", Verb::Take}, {"grab", Verb::Take}, {"pick", Verb::Take},
        {"use", Verb::Use}, {"apply", Verb::Use},
        {"read", Verb::Read},
        {"inventory", Verb::Inventory}, {"inv", Verb::Inventory}, {"i", Verb::Inventory},
        {"map", Verb::Map}, {"m", Verb::Map},
        {"codex", Verb::Codex}, {"logs", Verb::Codex},
        {"status", Verb::Status}, {"stat", Verb::Status},
        {"help", Verb::Help}, {"h", Verb::Help}, {"?", Verb::Help},
        {"save", Verb::Save},
        {"load", Verb::Load},
        {"quit", Verb::Quit}, {"exit", Verb::Quit}, {"q", Verb::Quit}
    };
    return table;
}

}

Verb Parser::resolveVerb(const std::string& token) const {
    const auto& table = verbTable();
    const auto exact = table.find(token);
    if (exact != table.end()) {
        return exact->second;
    }
    Verb prefixMatch = Verb::Unknown;
    int matches = 0;
    for (const auto& [word, verb] : table) {
        if (word.size() > token.size() && word.rfind(token, 0) == 0) {
            prefixMatch = verb;
            ++matches;
        }
    }
    return matches == 1 ? prefixMatch : Verb::Unknown;
}

Command Parser::parse(const std::string& rawInput) const {
    Command command;
    command.raw = strutil::trim(rawInput);
    const std::string normalized = strutil::toLower(command.raw);
    const std::vector<std::string> tokens = strutil::words(normalized);
    if (tokens.empty()) {
        command.verb = Verb::Empty;
        return command;
    }

    if (directionTokens().count(tokens.front()) != 0) {
        command.verb = Verb::Go;
        command.noun = canonicalDirection(tokens.front());
        return command;
    }

    command.verb = resolveVerb(tokens.front());

    if (command.verb == Verb::Use) {
        std::vector<std::string> nounParts;
        std::vector<std::string> targetParts;
        bool afterOn = false;
        for (std::size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "on" || tokens[i] == "with") {
                afterOn = true;
                continue;
            }
            (afterOn ? targetParts : nounParts).push_back(tokens[i]);
        }
        for (const auto& part : nounParts) {
            command.noun += command.noun.empty() ? part : " " + part;
        }
        for (const auto& part : targetParts) {
            command.target += command.target.empty() ? part : " " + part;
        }
        return command;
    }

    std::string remainder;
    for (std::size_t i = 1; i < tokens.size(); ++i) {
        remainder += remainder.empty() ? tokens[i] : " " + tokens[i];
    }
    if (command.verb == Verb::Go && !remainder.empty()) {
        remainder = canonicalDirection(remainder);
    }
    command.noun = remainder;
    return command;
}

}

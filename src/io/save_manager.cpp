#include "io/save_manager.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

#include "aeon/aeon.hpp"
#include "game_state.hpp"

namespace vestige {

namespace {

constexpr int SAVE_VERSION = 1;

int encodeState(LogState state) {
    return static_cast<int>(state);
}

int encodeVersion(LogVersion version) {
    return static_cast<int>(version);
}

LogState decodeState(int value) {
    switch (value) {
        case 1: return LogState::Corrupted;
        case 2: return LogState::Readable;
        default: return LogState::Sealed;
    }
}

LogVersion decodeVersion(int value) {
    switch (value) {
        case 1: return LogVersion::Original;
        case 2: return LogVersion::Revised;
        default: return LogVersion::Unchosen;
    }
}

}

bool SaveManager::save(const std::string& path, const GameState& state, const Aeon& aeon) const {
    nlohmann::json root;
    root["version"] = SAVE_VERSION;
    root["currentRoom"] = state.currentRoom;
    root["inventory"] = state.inventory();
    root["takenItems"] = std::vector<std::string>(state.takenItems().begin(), state.takenItems().end());
    root["visited"] = std::vector<std::string>(state.visitedRooms().begin(), state.visitedRooms().end());
    root["logsRead"] = std::vector<std::string>(state.logsRead().begin(), state.logsRead().end());
    root["solved"] = std::vector<std::string>(state.solvedPuzzles().begin(), state.solvedPuzzles().end());
    root["flags"] = std::vector<std::string>(state.flags().begin(), state.flags().end());

    nlohmann::json logStates = nlohmann::json::object();
    for (const auto& [id, logState] : state.discoveredLogs()) {
        logStates[id] = encodeState(logState);
    }
    root["logStates"] = logStates;

    nlohmann::json logVersions = nlohmann::json::object();
    for (const auto& [id, version] : state.logVersions()) {
        logVersions[id] = encodeVersion(version);
    }
    root["logVersions"] = logVersions;

    root["trust"] = aeon.trust();
    root["awareness"] = aeon.awareness();
    root["everQuestioned"] = aeon.everQuestioned();
    root["spokenAeon"] = std::vector<std::string>(aeon.spoken().begin(), aeon.spoken().end());

    std::ofstream stream(path);
    if (!stream.is_open()) {
        return false;
    }
    stream << root.dump(2);
    return stream.good();
}

LoadResult SaveManager::load(const std::string& path, GameState& state, Aeon& aeon) const {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        return LoadResult::Missing;
    }
    nlohmann::json root;
    try {
        stream >> root;
        state.currentRoom = root.at("currentRoom").get<std::string>();
        for (const auto& id : root.at("inventory")) {
            state.addItem(id.get<std::string>(), 9999);
        }
        for (const auto& id : root.at("takenItems")) {
            state.markItemTaken(id.get<std::string>());
        }
        for (const auto& id : root.at("visited")) {
            state.visitRoom(id.get<std::string>());
        }
        for (const auto& id : root.at("logsRead")) {
            state.markLogRead(id.get<std::string>());
        }
        for (const auto& id : root.at("solved")) {
            state.solvePuzzle(id.get<std::string>());
        }
        for (const auto& id : root.at("flags")) {
            state.setFlag(id.get<std::string>());
        }
        for (const auto& [id, value] : root.at("logStates").items()) {
            state.setLogState(id, decodeState(value.get<int>()));
        }
        for (const auto& [id, value] : root.at("logVersions").items()) {
            state.setLogVersion(id, decodeVersion(value.get<int>()));
        }
        aeon.setTrust(root.at("trust").get<int>());
        aeon.setAwareness(root.at("awareness").get<int>());
        aeon.setEverQuestioned(root.value("everQuestioned", false));
        for (const auto& id : root.at("spokenAeon")) {
            aeon.markSpoken(id.get<std::string>());
        }
    } catch (const std::exception&) {
        return LoadResult::Corrupt;
    }
    return LoadResult::Success;
}

}

#include "io/content_loader.hpp"

#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace vestige {

namespace {

std::optional<nlohmann::json> readJsonFile(const std::string& path, const FaultSink& fault) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        fault("cannot open resource file: " + path);
        return std::nullopt;
    }
    try {
        nlohmann::json root;
        stream >> root;
        return root;
    } catch (const std::exception& error) {
        fault("malformed json in " + path + ": " + error.what());
        return std::nullopt;
    }
}

ConditionType conditionTypeFromString(const std::string& text) {
    static const std::unordered_map<std::string, ConditionType> map = {
        {"puzzle", ConditionType::PuzzleSolved},
        {"log", ConditionType::LogRead},
        {"item", ConditionType::ItemHeld},
        {"flag", ConditionType::FlagSet},
        {"trust_above", ConditionType::TrustAbove},
        {"trust_below", ConditionType::TrustBelow},
        {"awareness_above", ConditionType::AwarenessAbove},
        {"awareness_below", ConditionType::AwarenessBelow}
    };
    const auto it = map.find(text);
    return it == map.end() ? ConditionType::None : it->second;
}

Condition parseCondition(const nlohmann::json& node) {
    Condition condition;
    if (!node.is_object()) {
        return condition;
    }
    condition.type = conditionTypeFromString(node.value("type", std::string{}));
    condition.targetId = node.value("value", std::string{});
    condition.threshold = node.value("threshold", 0);
    return condition;
}

ItemEffect itemEffectFromString(const std::string& text) {
    if (text == "flag") return ItemEffect::SetFlag;
    if (text == "aeon") return ItemEffect::AeonLine;
    if (text == "reveal") return ItemEffect::RevealLog;
    return ItemEffect::None;
}

LogState logStateFromString(const std::string& text) {
    if (text == "corrupted") return LogState::Corrupted;
    if (text == "sealed") return LogState::Sealed;
    return LogState::Readable;
}

Voice voiceFromString(const std::string& text) {
    if (text == "rewritten") return Voice::Rewritten;
    if (text == "system") return Voice::System;
    if (text == "error") return Voice::Error;
    if (text == "log") return Voice::Log;
    if (text == "room") return Voice::Room;
    return Voice::Aeon;
}

AeonTrigger aeonTriggerFromString(const std::string& text) {
    if (text == "log") return AeonTrigger::LogRead;
    if (text == "puzzle") return AeonTrigger::PuzzleSolved;
    if (text == "version") return AeonTrigger::VersionChosen;
    if (text == "awakening") return AeonTrigger::Awakening;
    return AeonTrigger::EnterRoom;
}

EndingId endingIdFromKey(const std::string& key) {
    if (key == "archivist") return EndingId::Archivist;
    if (key == "collaborator") return EndingId::Collaborator;
    if (key == "witness") return EndingId::Witness;
    if (key == "dissolution") return EndingId::Dissolution;
    return EndingId::Agnostic;
}

}

ContentLoader::ContentLoader(FaultSink fault) : fault_(std::move(fault)) {}

bool ContentLoader::loadRooms(const std::string& path, WorldGraph& world) {
    const auto root = readJsonFile(path, fault_);
    if (!root) {
        return false;
    }
    const auto& rooms = root->at("rooms");
    for (const auto& node : rooms) {
        Room room;
        room.id = node.at("id").get<std::string>();
        room.name = node.at("name").get<std::string>();
        room.sector = node.value("sector", std::string{});
        room.description = node.at("description").get<std::string>();
        room.aeonTagOnEnter = node.value("aeonTag", std::string{});
        room.terminus = node.value("terminus", false);
        for (const auto& exitNode : node.value("exits", nlohmann::json::array())) {
            Exit exit;
            exit.direction = exitNode.at("direction").get<std::string>();
            exit.targetId = exitNode.at("target").get<std::string>();
            exit.lockedMessage = exitNode.value("lockedMessage", std::string{});
            if (exitNode.contains("condition")) {
                exit.condition = parseCondition(exitNode.at("condition"));
            }
            room.exits.push_back(std::move(exit));
        }
        room.logIds = node.value("logs", std::vector<std::string>{});
        room.puzzleIds = node.value("puzzles", std::vector<std::string>{});
        world.addRoom(std::move(room));
    }
    world.setStartRoom(root->value("start", std::string{}));
    return true;
}

bool ContentLoader::loadItems(const std::string& path, WorldGraph& world) {
    const auto root = readJsonFile(path, fault_);
    if (!root) {
        return false;
    }
    for (const auto& node : root->at("items")) {
        Item item;
        item.id = node.at("id").get<std::string>();
        item.name = node.at("name").get<std::string>();
        item.description = node.at("description").get<std::string>();
        item.effect = itemEffectFromString(node.value("effect", std::string{}));
        item.effectTarget = node.value("effectTarget", std::string{});
        item.useMessage = node.value("useMessage", std::string{});
        item.startRoom = node.value("startRoom", std::string{});
        world.addItem(std::move(item));
    }
    return true;
}

bool ContentLoader::loadLogs(const std::string& path, WorldGraph& world) {
    const auto root = readJsonFile(path, fault_);
    if (!root) {
        return false;
    }
    for (const auto& node : root->at("logs")) {
        LogEntry log;
        log.id = node.at("id").get<std::string>();
        log.title = node.at("title").get<std::string>();
        log.category = node.value("category", std::string{});
        log.initialState = logStateFromString(node.value("state", std::string{"readable"}));
        log.repairPuzzleId = node.value("repairPuzzle", std::string{});
        log.original = node.at("original").get<std::string>();
        if (node.contains("revised")) {
            log.revised = node.at("revised").get<std::string>();
            log.hasRevised = true;
        }
        log.roomId = node.value("room", std::string{});
        log.unlocksFlag = node.value("unlocksFlag", std::string{});
        log.aeonTagOnRead = node.value("aeonTag", std::string{});
        world.addLog(std::move(log));
    }
    return true;
}

bool ContentLoader::loadAeon(const std::string& path, std::vector<AeonResponse>& lines) {
    const auto root = readJsonFile(path, fault_);
    if (!root) {
        return false;
    }
    for (const auto& node : root->at("lines")) {
        AeonResponse response;
        response.id = node.at("id").get<std::string>();
        response.trigger = aeonTriggerFromString(node.value("trigger", std::string{"enter"}));
        response.triggerValue = node.value("value", std::string{});
        response.minTrust = node.value("minTrust", -1000);
        response.maxTrust = node.value("maxTrust", 1000);
        response.minAwareness = node.value("minAwareness", -1000);
        response.maxAwareness = node.value("maxAwareness", 1000);
        response.line = node.at("line").get<std::string>();
        response.voice = voiceFromString(node.value("voice", std::string{"aeon"}));
        response.once = node.value("once", true);
        lines.push_back(std::move(response));
    }
    return true;
}

bool ContentLoader::loadEndings(const std::string& path, std::vector<Ending>& endings) {
    const auto root = readJsonFile(path, fault_);
    if (!root) {
        return false;
    }
    for (const auto& node : root->at("endings")) {
        Ending ending;
        ending.key = node.at("key").get<std::string>();
        ending.id = endingIdFromKey(ending.key);
        ending.title = node.at("title").get<std::string>();
        ending.paragraphs = node.at("paragraphs").get<std::vector<std::string>>();
        ending.aeonLine = node.at("aeonLine").get<std::string>();
        endings.push_back(std::move(ending));
    }
    return true;
}

bool ContentLoader::validate(const LoadedContent& content) {
    const WorldGraph& world = content.world;
    if (world.room(world.startRoom()) == nullptr) {
        fault_("start room is missing or undefined: " + world.startRoom());
        return false;
    }
    for (const std::string& roomId : world.roomOrder()) {
        const Room* room = world.room(roomId);
        for (const Exit& exit : room->exits) {
            if (world.room(exit.targetId) == nullptr) {
                fault_("exit target missing: " + exit.targetId + " from " + roomId);
                return false;
            }
        }
        for (const std::string& logId : room->logIds) {
            if (world.log(logId) == nullptr) {
                fault_("room references unknown log: " + logId);
                return false;
            }
        }
        for (const std::string& puzzleId : room->puzzleIds) {
            if (content.puzzles.get(puzzleId) == nullptr) {
                fault_("room references unknown puzzle: " + puzzleId);
                return false;
            }
        }
    }
    for (const std::string& logId : world.logOrder()) {
        const LogEntry* log = world.log(logId);
        if (!log->repairPuzzleId.empty() && content.puzzles.get(log->repairPuzzleId) == nullptr) {
            fault_("log references unknown repair puzzle: " + log->repairPuzzleId);
            return false;
        }
    }
    return true;
}

std::optional<LoadedContent> ContentLoader::load(const std::string& resourceDir) {
    LoadedContent content;
    const std::string prefix = resourceDir + "/";

    try {
        const auto puzzleJson = readJsonFile(prefix + "puzzles.json", fault_);
        if (!puzzleJson || !content.puzzles.loadFromJson(*puzzleJson, fault_)) {
            return std::nullopt;
        }
        if (!loadRooms(prefix + "rooms.json", content.world)) return std::nullopt;
        if (!loadItems(prefix + "items.json", content.world)) return std::nullopt;
        if (!loadLogs(prefix + "logs.json", content.world)) return std::nullopt;
        if (!loadAeon(prefix + "aeon_lines.json", content.aeonLines)) return std::nullopt;
        if (!loadEndings(prefix + "endings.json", content.endings)) return std::nullopt;
    } catch (const std::exception& error) {
        fault_(std::string{"content field error: "} + error.what());
        return std::nullopt;
    }
    if (!validate(content)) {
        return std::nullopt;
    }
    return content;
}

}

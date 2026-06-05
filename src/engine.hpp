#pragma once

#include <string>
#include <vector>

#include "aeon/aeon.hpp"
#include "command.hpp"
#include "config/config.hpp"
#include "endings.hpp"
#include "game_state.hpp"
#include "io/content_loader.hpp"
#include "io/save_manager.hpp"
#include "parser.hpp"
#include "puzzle/puzzle_registry.hpp"
#include "terminal.hpp"
#include "world/world_graph.hpp"

namespace vestige {

class Engine {
public:
    Engine(Config config, Terminal& terminal, LoadedContent content);

    void run();

private:
    void startNewGame();
    void enterRoom(const std::string& id);
    void describeRoom(bool full);
    void handle(const Command& command);

    void cmdGo(const Command& command);
    void cmdExamine(const Command& command);
    void cmdTake(const Command& command);
    void cmdUse(const Command& command);
    void cmdRead(const Command& command);
    void cmdInventory();
    void cmdMap();
    void cmdCodex();
    void cmdStatus();
    void cmdHelp();
    void cmdSave();
    void cmdLoad();

    bool conditionMet(const Condition& condition) const;
    void fireAeon(AeonTrigger trigger, const std::string& value);
    bool runPuzzle(const Puzzle& puzzle);
    void chooseLogVersion(const LogEntry& log);
    void revealLogContent(const LogEntry& log);
    void applyLogEffects(const LogEntry& log);
    LogState displayStateFor(const LogEntry& log) const;
    void triggerEnding();

    const Item* findRoomItem(const std::string& query) const;
    const Item* findInventoryItem(const std::string& query) const;
    const LogEntry* findReadableLog(const std::string& query) const;
    const Ending* findEnding(EndingId id) const;

    Config config_;
    Terminal& terminal_;
    Parser parser_;
    WorldGraph world_;
    PuzzleRegistry puzzles_;
    std::vector<Ending> endings_;
    Aeon aeon_;
    GameState state_;
    SaveManager saver_;
    bool running_ = true;
};

}

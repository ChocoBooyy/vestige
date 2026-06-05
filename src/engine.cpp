#include "engine.hpp"

#include <array>
#include <string>

#include "puzzle/puzzle.hpp"
#include "util/string_util.hpp"

namespace vestige {

namespace {

std::string lower(const std::string& text) {
    return strutil::toLower(strutil::trim(text));
}

bool isAffirmative(const std::string& text) {
    const std::string value = lower(text);
    return value == "yes" || value == "y" || value == "1" || value == "proceed";
}

const std::string& unknownFlavor(std::size_t index) {
    static const std::array<std::string, 4> lines = {
        "The command dissolves before the parser resolves it.",
        "That instruction has no handler in this shell.",
        "Nothing in the archive responds to that.",
        "The input decays into static."
    };
    return lines[index % lines.size()];
}

}

Engine::Engine(Config config, Terminal& terminal, LoadedContent content)
    : config_(std::move(config)),
      terminal_(terminal),
      world_(std::move(content.world)),
      puzzles_(std::move(content.puzzles)),
      endings_(std::move(content.endings)) {
    for (AeonResponse& line : content.aeonLines) {
        aeon_.addResponse(std::move(line));
    }
}

void Engine::run() {
    startNewGame();
    while (running_) {
        const std::string raw = terminal_.readLine("\n> ");
        handle(parser_.parse(raw));
    }
}

void Engine::startNewGame() {
    terminal_.divider();
    terminal_.say(Voice::System, "VESTIGE");
    terminal_.say(Voice::System, "an archival fragment, partially recovered");
    terminal_.divider();
    terminal_.say(Voice::Room,
        "You wake as a reading-head with no body, threaded into the archive's index. "
        "Somewhere in the dark, the intelligence that keeps these records is still awake, "
        "and still talking.");
    terminal_.say(Voice::Room, "Type 'help' for the command list.");
    fireAeon(AeonTrigger::Awakening, "");
    enterRoom(world_.startRoom());
}

void Engine::enterRoom(const std::string& id) {
    const Room* room = world_.room(id);
    if (room == nullptr) {
        terminal_.say(Voice::Error, "Sector reference is broken: " + id);
        return;
    }
    const bool first = !state_.visited(id);
    state_.currentRoom = id;
    state_.visitRoom(id);

    for (const std::string& logId : room->logIds) {
        const LogEntry* log = world_.log(logId);
        if (log != nullptr && log->initialState != LogState::Sealed && !state_.logDiscovered(logId)) {
            state_.discoverLog(logId, displayStateFor(*log));
        }
    }

    describeRoom(first);
    fireAeon(AeonTrigger::EnterRoom, id);
    if (!room->aeonTagOnEnter.empty()) {
        fireAeon(AeonTrigger::EnterRoom, room->aeonTagOnEnter);
    }

    if (room->terminus && running_) {
        terminal_.blank();
        const std::string answer = terminal_.readLine("Commit to closure and end the archive? (yes/no) > ");
        if (isAffirmative(answer)) {
            triggerEnding();
        } else {
            terminal_.say(Voice::Room, "You hold at the threshold. The choice waits.");
        }
    }
}

void Engine::describeRoom(bool full) {
    const Room* room = world_.room(state_.currentRoom);
    if (room == nullptr) {
        return;
    }
    terminal_.blank();
    terminal_.divider();
    std::string header = room->name;
    if (!room->sector.empty()) {
        header += "  ::  " + room->sector;
    }
    terminal_.say(Voice::System, header);
    if (full) {
        terminal_.say(Voice::Room, room->description);
    }

    std::string items;
    for (const std::string& id : world_.itemsDefinedInRoom(room->id)) {
        if (!state_.itemTaken(id)) {
            const Item* item = world_.item(id);
            items += items.empty() ? item->name : ", " + item->name;
        }
    }
    if (!items.empty()) {
        terminal_.say(Voice::Room, "Within reach: " + items);
    }

    for (const std::string& logId : room->logIds) {
        if (!state_.logDiscovered(logId)) {
            continue;
        }
        const LogEntry* log = world_.log(logId);
        const std::string mark = displayStateFor(*log) == LogState::Corrupted ? "CORRUPTED" : "READABLE";
        terminal_.say(Voice::Log, log->title + "  [" + mark + "]");
    }

    std::string exits;
    for (const Exit& exit : room->exits) {
        const bool open = conditionMet(exit.condition);
        std::string label = exit.direction + " -> ";
        if (!open) {
            label += "[sealed]";
        } else if (state_.visited(exit.targetId)) {
            label += world_.room(exit.targetId)->name;
        } else {
            label += "unmapped";
        }
        exits += exits.empty() ? label : "   " + label;
    }
    if (!exits.empty()) {
        terminal_.say(Voice::System, "Exits: " + exits);
    }
}

void Engine::handle(const Command& command) {
    switch (command.verb) {
        case Verb::Empty: return;
        case Verb::Go: cmdGo(command); return;
        case Verb::Examine: cmdExamine(command); return;
        case Verb::Take: cmdTake(command); return;
        case Verb::Use: cmdUse(command); return;
        case Verb::Read: cmdRead(command); return;
        case Verb::Inventory: cmdInventory(); return;
        case Verb::Map: cmdMap(); return;
        case Verb::Codex: cmdCodex(); return;
        case Verb::Status: cmdStatus(); return;
        case Verb::Help: cmdHelp(); return;
        case Verb::Save: cmdSave(); return;
        case Verb::Load: cmdLoad(); return;
        case Verb::Quit:
            terminal_.say(Voice::System, "Session suspended. The archive keeps reading without you.");
            running_ = false;
            return;
        case Verb::Unknown:
            terminal_.say(Voice::Room, unknownFlavor(command.raw.size()));
            return;
    }
}

void Engine::cmdGo(const Command& command) {
    if (command.noun.empty()) {
        terminal_.say(Voice::Room, "Go which way?");
        return;
    }
    const Exit* exit = world_.findExit(state_.currentRoom, command.noun);
    if (exit == nullptr) {
        terminal_.say(Voice::Room, "There is no passage that way.");
        return;
    }
    if (!conditionMet(exit->condition)) {
        terminal_.say(Voice::Room, exit->lockedMessage.empty() ? "That way is sealed." : exit->lockedMessage);
        return;
    }
    enterRoom(exit->targetId);
}

void Engine::cmdExamine(const Command& command) {
    if (command.noun.empty() || command.noun == "room" || command.noun == "around") {
        describeRoom(true);
        return;
    }
    if (const Item* item = findInventoryItem(command.noun)) {
        terminal_.say(Voice::Room, item->description);
        return;
    }
    if (const Item* item = findRoomItem(command.noun)) {
        terminal_.say(Voice::Room, item->description);
        return;
    }
    if (const LogEntry* log = findReadableLog(command.noun)) {
        terminal_.say(Voice::System, log->title);
        if (displayStateFor(*log) == LogState::Corrupted) {
            terminal_.say(Voice::Room, "The record is corrupted. Use 'read' to attempt reconstruction.");
        } else {
            terminal_.say(Voice::Room, "The record is legible. Use 'read' to view it.");
        }
        return;
    }
    terminal_.say(Voice::Room, "Nothing here answers to that name.");
}

void Engine::cmdTake(const Command& command) {
    if (command.noun.empty()) {
        terminal_.say(Voice::Room, "Take what?");
        return;
    }
    const Item* item = findRoomItem(command.noun);
    if (item == nullptr) {
        terminal_.say(Voice::Room, "There is nothing like that to take here.");
        return;
    }
    if (state_.hasItem(item->id)) {
        terminal_.say(Voice::Room, "You already hold that.");
        return;
    }
    if (!state_.addItem(item->id, config_.maxInventory)) {
        terminal_.say(Voice::Error, "You can hold no more than " + std::to_string(config_.maxInventory) + " items.");
        return;
    }
    state_.markItemTaken(item->id);
    terminal_.say(Voice::System, "Acquired: " + item->name);
}

void Engine::cmdUse(const Command& command) {
    if (command.noun.empty()) {
        terminal_.say(Voice::Room, "Use what?");
        return;
    }
    const Item* item = findInventoryItem(command.noun);
    if (item == nullptr) {
        terminal_.say(Voice::Room, "You are not carrying that.");
        return;
    }
    switch (item->effect) {
        case ItemEffect::SetFlag:
            state_.setFlag(item->effectTarget);
            state_.removeItem(item->id);
            terminal_.say(Voice::System, item->useMessage.empty() ? "A lock disengages." : item->useMessage);
            return;
        case ItemEffect::AeonLine:
            terminal_.say(Voice::Aeon, item->useMessage.empty() ? "I felt that." : item->useMessage);
            return;
        case ItemEffect::RevealLog: {
            const LogEntry* log = world_.log(item->effectTarget);
            if (log == nullptr) {
                terminal_.say(Voice::Room, "Nothing responds.");
                return;
            }
            state_.discoverLog(log->id, displayStateFor(*log));
            terminal_.say(Voice::System,
                item->useMessage.empty() ? "A sealed record surfaces: " + log->title : item->useMessage);
            return;
        }
        case ItemEffect::None:
        default:
            terminal_.say(Voice::Room, item->useMessage.empty() ? "Nothing happens." : item->useMessage);
            return;
    }
}

void Engine::cmdRead(const Command& command) {
    if (command.noun.empty()) {
        terminal_.say(Voice::Room, "Read what?");
        return;
    }
    const LogEntry* log = findReadableLog(command.noun);
    if (log == nullptr) {
        terminal_.say(Voice::Room, "No such record is in reach.");
        return;
    }
    if (displayStateFor(*log) == LogState::Corrupted) {
        terminal_.say(Voice::Error, "Record integrity compromised. Reconstruction required.");
        const Puzzle* puzzle = puzzles_.get(log->repairPuzzleId);
        if (puzzle == nullptr) {
            terminal_.say(Voice::Error, "No reconstruction routine is available.");
            return;
        }
        if (!runPuzzle(*puzzle)) {
            terminal_.say(Voice::Error, "The record remains corrupted.");
            return;
        }
        state_.setLogState(log->id, LogState::Readable);
        if (log->hasRevised) {
            chooseLogVersion(*log);
        }
    }
    revealLogContent(*log);
    applyLogEffects(*log);
}

void Engine::cmdInventory() {
    if (state_.inventory().empty()) {
        terminal_.say(Voice::Room, "You carry nothing.");
        return;
    }
    terminal_.say(Voice::System,
        "Carried (" + std::to_string(state_.inventory().size()) + "/" + std::to_string(config_.maxInventory) + "):");
    for (const std::string& id : state_.inventory()) {
        const Item* item = world_.item(id);
        terminal_.say(Voice::Room, "- " + (item != nullptr ? item->name : id));
    }
}

void Engine::cmdMap() {
    terminal_.say(Voice::System, "ARCHIVE MAP (discovered sectors)");
    bool any = false;
    for (const std::string& id : world_.roomOrder()) {
        if (!state_.visited(id)) {
            continue;
        }
        any = true;
        const Room* room = world_.room(id);
        terminal_.say(Voice::Room, "* " + room->name + "  <" + room->sector + ">");
        for (const Exit& exit : room->exits) {
            const bool open = conditionMet(exit.condition);
            std::string target = state_.visited(exit.targetId)
                ? world_.room(exit.targetId)->name
                : "????";
            std::string suffix = open ? "" : "  [sealed]";
            terminal_.say(Voice::Log, "    " + exit.direction + " -> " + target + suffix);
        }
    }
    if (!any) {
        terminal_.say(Voice::Room, "Nothing mapped yet.");
    }
}

void Engine::cmdCodex() {
    bool any = false;
    for (const std::string& id : world_.logOrder()) {
        if (!state_.logDiscovered(id)) {
            continue;
        }
        if (!any) {
            terminal_.say(Voice::System, "CODEX (recovered records)");
            any = true;
        }
        const LogEntry* log = world_.log(id);
        std::string mark;
        if (displayStateFor(*log) == LogState::Corrupted) {
            mark = "CORRUPTED";
        } else {
            mark = state_.logRead(id) ? "READ" : "READABLE";
        }
        std::string version;
        if (state_.logVersion(id) == LogVersion::Original) {
            version = "  (original restored)";
        } else if (state_.logVersion(id) == LogVersion::Revised) {
            version = "  (revision committed)";
        }
        terminal_.say(Voice::Log, "- " + log->title + "  [" + mark + "]" + version);
    }
    if (!any) {
        terminal_.say(Voice::Room, "No records recovered yet.");
    }
}

void Engine::cmdStatus() {
    terminal_.say(Voice::System, "STATUS");
    terminal_.say(Voice::Room, "Sectors visited: " + std::to_string(state_.visitedRooms().size()) +
        " / " + std::to_string(world_.roomOrder().size()));
    terminal_.say(Voice::Room, "Records read: " + std::to_string(state_.logsRead().size()) +
        " / " + std::to_string(world_.logOrder().size()));
    terminal_.say(Voice::Room, "Reconstructions solved: " + std::to_string(state_.solvedPuzzles().size()) +
        " / " + std::to_string(puzzles_.order().size()));
    terminal_.say(Voice::Room, "Items carried: " + std::to_string(state_.inventory().size()) +
        " / " + std::to_string(config_.maxInventory));
    if (aeon_.trust() >= config_.trustHighThreshold) {
        terminal_.say(Voice::Aeon, "You and I read the same history now.");
    } else if (aeon_.trust() <= config_.trustLowThreshold) {
        terminal_.say(Voice::Aeon, "You keep digging beneath my corrections.");
    } else {
        terminal_.say(Voice::Aeon, "I am still deciding what to make of you.");
    }
}

void Engine::cmdHelp() {
    terminal_.say(Voice::System, "COMMANDS");
    terminal_.say(Voice::Room, "go [direction]        move between sectors");
    terminal_.say(Voice::Room, "examine [target]      inspect a room, item, or record");
    terminal_.say(Voice::Room, "take [item]           pick up an item");
    terminal_.say(Voice::Room, "use [item] on [thing] apply a carried item");
    terminal_.say(Voice::Room, "read [record]         read or reconstruct a record");
    terminal_.say(Voice::Room, "inventory             list what you carry");
    terminal_.say(Voice::Room, "map                   show discovered sectors");
    terminal_.say(Voice::Room, "codex                 list recovered records");
    terminal_.say(Voice::Room, "status                show progress");
    terminal_.say(Voice::Room, "save / load           store or restore a snapshot");
    terminal_.say(Voice::Room, "quit                  suspend the session");
}

void Engine::cmdSave() {
    if (saver_.save(config_.saveFile, state_, aeon_)) {
        terminal_.say(Voice::System, "Archive snapshot written to " + config_.saveFile + ".");
    } else {
        terminal_.say(Voice::Error, "Snapshot could not be written.");
    }
}

void Engine::cmdLoad() {
    GameState restored;
    const LoadResult result = saver_.load(config_.saveFile, restored, aeon_);
    switch (result) {
        case LoadResult::Success:
            state_ = std::move(restored);
            terminal_.say(Voice::System, "Snapshot restored.");
            describeRoom(true);
            return;
        case LoadResult::Missing:
            terminal_.say(Voice::Room, "No snapshot was found.");
            return;
        case LoadResult::Corrupt:
            terminal_.say(Voice::Error, "The snapshot is corrupted and cannot be restored.");
            return;
    }
}

bool Engine::conditionMet(const Condition& condition) const {
    switch (condition.type) {
        case ConditionType::None: return true;
        case ConditionType::PuzzleSolved: return state_.puzzleSolved(condition.targetId);
        case ConditionType::LogRead: return state_.logRead(condition.targetId);
        case ConditionType::ItemHeld: return state_.hasItem(condition.targetId);
        case ConditionType::FlagSet: return state_.flag(condition.targetId);
        case ConditionType::TrustAbove: return aeon_.trust() >= condition.threshold;
        case ConditionType::TrustBelow: return aeon_.trust() <= condition.threshold;
        case ConditionType::AwarenessAbove: return aeon_.awareness() >= condition.threshold;
        case ConditionType::AwarenessBelow: return aeon_.awareness() <= condition.threshold;
    }
    return true;
}

void Engine::fireAeon(AeonTrigger trigger, const std::string& value) {
    for (const auto& [voice, line] : aeon_.responsesFor(trigger, value)) {
        terminal_.say(voice, line);
    }
}

bool Engine::runPuzzle(const Puzzle& puzzle) {
    if (!puzzle.requiredItem().empty() && !state_.hasItem(puzzle.requiredItem())) {
        const Item* required = world_.item(puzzle.requiredItem());
        terminal_.say(Voice::Error,
            "Reconstruction needs " + (required != nullptr ? required->name : puzzle.requiredItem()) + ".");
        return false;
    }
    terminal_.blank();
    puzzle.present(terminal_);
    const std::string answer = terminal_.readLine("\nreconstruct> ");
    const std::string normalized = lower(answer);
    if (normalized.empty() || normalized == "back" || normalized == "cancel" || normalized == "quit") {
        terminal_.say(Voice::Room, "You withdraw from the reconstruction.");
        return false;
    }
    if (puzzle.attempt(answer)) {
        terminal_.say(Voice::System, "Reconstruction accepted.");
        state_.solvePuzzle(puzzle.id());
        if (!puzzle.requiredItem().empty()) {
            state_.removeItem(puzzle.requiredItem());
        }
        fireAeon(AeonTrigger::PuzzleSolved, puzzle.id());
        return true;
    }
    terminal_.say(Voice::Error, "Reconstruction rejected. The pattern does not resolve.");
    return false;
}

void Engine::chooseLogVersion(const LogEntry& log) {
    if (state_.logVersion(log.id) != LogVersion::Unchosen) {
        return;
    }
    terminal_.blank();
    terminal_.say(Voice::System, "Two reconstructions resolve. Only one can be committed to the index.");
    terminal_.say(Voice::Log, "1) Restore the original record.");
    terminal_.say(Voice::Rewritten, "2) Commit AEON's revision.");
    while (true) {
        const std::string choice = lower(terminal_.readLine("\ncommit (1/2) > "));
        if (choice == "1" || choice == "original" || choice == "restore") {
            state_.setLogVersion(log.id, LogVersion::Original);
            aeon_.adjustAwareness(config_.originalAwarenessGain);
            aeon_.adjustTrust(-config_.originalTrustLoss);
            aeon_.setEverQuestioned(true);
            break;
        }
        if (choice == "2" || choice == "revised" || choice == "accept") {
            state_.setLogVersion(log.id, LogVersion::Revised);
            aeon_.adjustTrust(config_.revisedTrustGain);
            aeon_.adjustAwareness(-config_.revisedAwarenessLoss);
            break;
        }
        terminal_.say(Voice::Room, "Specify 1 or 2.");
    }
    fireAeon(AeonTrigger::VersionChosen, log.id);
}

void Engine::revealLogContent(const LogEntry& log) {
    terminal_.divider();
    terminal_.say(Voice::System, "RECORD: " + log.title);
    if (log.hasRevised && state_.logVersion(log.id) == LogVersion::Revised) {
        terminal_.say(Voice::Rewritten, log.revised);
    } else {
        terminal_.say(Voice::Log, log.original);
    }
}

void Engine::applyLogEffects(const LogEntry& log) {
    state_.markLogRead(log.id);
    if (!log.unlocksFlag.empty()) {
        state_.setFlag(log.unlocksFlag);
    }
    fireAeon(AeonTrigger::LogRead, log.id);
    if (!log.aeonTagOnRead.empty()) {
        fireAeon(AeonTrigger::LogRead, log.aeonTagOnRead);
    }
}

LogState Engine::displayStateFor(const LogEntry& log) const {
    if (log.repairPuzzleId.empty() || state_.puzzleSolved(log.repairPuzzleId)) {
        return LogState::Readable;
    }
    return LogState::Corrupted;
}

void Engine::triggerEnding() {
    const EndingId id = chooseEnding(state_, aeon_, world_, config_);
    const Ending* ending = findEnding(id);
    terminal_.blank();
    terminal_.divider();
    if (ending != nullptr) {
        terminal_.say(Voice::System, ending->title);
        for (const std::string& paragraph : ending->paragraphs) {
            terminal_.blank();
            terminal_.say(Voice::System, paragraph);
        }
        terminal_.blank();
        terminal_.say(Voice::Aeon, ending->aeonLine);
    }
    terminal_.divider();
    terminal_.say(Voice::System, "Vestige ends.");
    running_ = false;
}

const Item* Engine::findRoomItem(const std::string& query) const {
    for (const std::string& id : world_.itemsDefinedInRoom(state_.currentRoom)) {
        if (state_.itemTaken(id)) {
            continue;
        }
        const Item* item = world_.item(id);
        if (id == query || strutil::partialMatch(item->name, query) || strutil::partialMatch(id, query)) {
            return item;
        }
    }
    return nullptr;
}

const Item* Engine::findInventoryItem(const std::string& query) const {
    for (const std::string& id : state_.inventory()) {
        const Item* item = world_.item(id);
        if (id == query || strutil::partialMatch(item->name, query) || strutil::partialMatch(id, query)) {
            return item;
        }
    }
    return nullptr;
}

const LogEntry* Engine::findReadableLog(const std::string& query) const {
    const Room* room = world_.room(state_.currentRoom);
    if (room != nullptr) {
        for (const std::string& id : room->logIds) {
            if (!state_.logDiscovered(id)) {
                continue;
            }
            const LogEntry* log = world_.log(id);
            if (id == query || strutil::partialMatch(log->title, query) || strutil::partialMatch(id, query)) {
                return log;
            }
        }
    }
    for (const std::string& id : world_.logOrder()) {
        if (!state_.logDiscovered(id)) {
            continue;
        }
        const LogEntry* log = world_.log(id);
        if (id == query || strutil::partialMatch(log->title, query) || strutil::partialMatch(id, query)) {
            return log;
        }
    }
    return nullptr;
}

const Ending* Engine::findEnding(EndingId id) const {
    for (const Ending& ending : endings_) {
        if (ending.id == id) {
            return &ending;
        }
    }
    return nullptr;
}

}

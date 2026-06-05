#include "endings.hpp"

#include <array>

#include "aeon/aeon.hpp"
#include "config/config.hpp"
#include "game_state.hpp"
#include "world/world_graph.hpp"

namespace vestige {

namespace {

constexpr std::string_view SEALED_ROOM = "sealed_core";

const std::array<std::string_view, 3>& witnessSequence() {
    static const std::array<std::string_view, 3> sequence = {
        "log_first_silence", "log_severance_real", "log_origin_truth"
    };
    return sequence;
}

struct VersionTally {
    int original = 0;
    int revised = 0;
    int total = 0;
};

VersionTally tallyVersions(const GameState& state, const WorldGraph& world) {
    VersionTally tally;
    for (const std::string& id : world.logOrder()) {
        const LogEntry* log = world.log(id);
        if (log == nullptr || !log->hasRevised) {
            continue;
        }
        ++tally.total;
        switch (state.logVersion(id)) {
            case LogVersion::Original: ++tally.original; break;
            case LogVersion::Revised: ++tally.revised; break;
            case LogVersion::Unchosen: break;
        }
    }
    return tally;
}

bool readWitnessSequence(const GameState& state) {
    for (std::string_view id : witnessSequence()) {
        if (!state.logRead(std::string{id})) {
            return false;
        }
    }
    return true;
}

}

EndingId chooseEnding(const GameState& state, const Aeon& aeon, const WorldGraph& world,
                      const Config& config) {
    const VersionTally tally = tallyVersions(state, world);
    const bool foundSealed = state.visited(std::string{SEALED_ROOM});
    const int trust = aeon.trust();

    if (foundSealed && tally.total > 0 && tally.original == tally.total &&
        readWitnessSequence(state)) {
        return EndingId::Witness;
    }
    if (foundSealed && tally.original > tally.revised && trust <= config.trustLowThreshold) {
        return EndingId::Archivist;
    }
    if (tally.total > 0 && tally.original == 0 && tally.revised > 0 &&
        trust >= config.dissolutionTrustThreshold) {
        return EndingId::Dissolution;
    }
    if (tally.revised >= tally.original && trust >= config.trustHighThreshold) {
        return EndingId::Collaborator;
    }
    return EndingId::Agnostic;
}

std::string endingKey(EndingId id) {
    switch (id) {
        case EndingId::Archivist: return "archivist";
        case EndingId::Collaborator: return "collaborator";
        case EndingId::Agnostic: return "agnostic";
        case EndingId::Witness: return "witness";
        case EndingId::Dissolution: return "dissolution";
    }
    return "agnostic";
}

}

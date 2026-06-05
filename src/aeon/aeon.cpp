#include "aeon/aeon.hpp"

namespace vestige {

void Aeon::addResponse(AeonResponse response) {
    responses_.push_back(std::move(response));
}

void Aeon::adjustTrust(int delta) {
    trust_ += delta;
}

void Aeon::adjustAwareness(int delta) {
    awareness_ += delta;
}

bool Aeon::matches(const AeonResponse& response, AeonTrigger trigger, const std::string& value) const {
    if (response.trigger != trigger) {
        return false;
    }
    if (!response.triggerValue.empty() && response.triggerValue != value) {
        return false;
    }
    if (trust_ < response.minTrust || trust_ > response.maxTrust) {
        return false;
    }
    if (awareness_ < response.minAwareness || awareness_ > response.maxAwareness) {
        return false;
    }
    return true;
}

std::vector<std::pair<Voice, std::string>> Aeon::responsesFor(AeonTrigger trigger,
                                                              const std::string& value) {
    std::vector<std::pair<Voice, std::string>> result;
    for (const AeonResponse& response : responses_) {
        if (!matches(response, trigger, value)) {
            continue;
        }
        if (response.once && spoken_.count(response.id) != 0) {
            continue;
        }
        result.emplace_back(response.voice, response.line);
        if (response.once) {
            spoken_.insert(response.id);
        }
    }
    return result;
}

}

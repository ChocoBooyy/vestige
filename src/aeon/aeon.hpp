#pragma once

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "aeon/aeon_response.hpp"

namespace vestige {

class Aeon {
public:
    void addResponse(AeonResponse response);

    void adjustTrust(int delta);
    void adjustAwareness(int delta);

    int trust() const { return trust_; }
    int awareness() const { return awareness_; }
    void setTrust(int value) { trust_ = value; }
    void setAwareness(int value) { awareness_ = value; }

    std::vector<std::pair<Voice, std::string>> responsesFor(AeonTrigger trigger,
                                                            const std::string& value);

    const std::unordered_set<std::string>& spoken() const { return spoken_; }
    void markSpoken(const std::string& id) { spoken_.insert(id); }
    bool everQuestioned() const { return everQuestioned_; }
    void setEverQuestioned(bool value) { everQuestioned_ = value; }

private:
    bool matches(const AeonResponse& response, AeonTrigger trigger, const std::string& value) const;

    std::vector<AeonResponse> responses_;
    std::unordered_set<std::string> spoken_;
    int trust_ = 0;
    int awareness_ = 0;
    bool everQuestioned_ = false;
};

}

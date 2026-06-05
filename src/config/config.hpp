#pragma once

#include <string>

namespace vestige {

struct Config {
    int typewriterMsPerChar = 16;
    int aeonMsPerChar = 26;
    int dividerWidth = 60;
    int maxInventory = 12;

    int trustHighThreshold = 6;
    int trustLowThreshold = 2;
    int dissolutionTrustThreshold = 9;
    int awarenessSealedThreshold = 6;

    int originalAwarenessGain = 2;
    int originalTrustLoss = 1;
    int revisedTrustGain = 2;
    int revisedAwarenessLoss = 1;

    bool ansiEnabled = true;
    std::string saveFile = "vestige_save.json";
    std::string resourceDir = "resources";

    static Config loadOrDefault(const std::string& path);
};

}

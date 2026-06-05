#include "config/config.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

namespace vestige {

namespace {

template <typename T>
void readField(const nlohmann::json& root, const char* key, T& target) {
    if (root.contains(key)) {
        target = root.at(key).get<T>();
    }
}

}

Config Config::loadOrDefault(const std::string& path) {
    Config config;
    std::ifstream stream(path);
    if (!stream.is_open()) {
        return config;
    }
    nlohmann::json root;
    try {
        stream >> root;
    } catch (const std::exception&) {
        return config;
    }
    readField(root, "typewriterMsPerChar", config.typewriterMsPerChar);
    readField(root, "aeonMsPerChar", config.aeonMsPerChar);
    readField(root, "dividerWidth", config.dividerWidth);
    readField(root, "maxInventory", config.maxInventory);
    readField(root, "trustHighThreshold", config.trustHighThreshold);
    readField(root, "trustLowThreshold", config.trustLowThreshold);
    readField(root, "dissolutionTrustThreshold", config.dissolutionTrustThreshold);
    readField(root, "awarenessSealedThreshold", config.awarenessSealedThreshold);
    readField(root, "originalAwarenessGain", config.originalAwarenessGain);
    readField(root, "originalTrustLoss", config.originalTrustLoss);
    readField(root, "revisedTrustGain", config.revisedTrustGain);
    readField(root, "revisedAwarenessLoss", config.revisedAwarenessLoss);
    readField(root, "ansiEnabled", config.ansiEnabled);
    readField(root, "saveFile", config.saveFile);
    readField(root, "resourceDir", config.resourceDir);
    return config;
}

}

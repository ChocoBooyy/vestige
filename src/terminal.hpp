#pragma once

#include <string>

#include "config/config.hpp"

namespace vestige {

enum class Voice {
    Room,
    Log,
    Aeon,
    Rewritten,
    Error,
    System
};

class Terminal {
public:
    Terminal(const Config& config, bool ansiActive);
    ~Terminal();

    void say(Voice voice, const std::string& text);
    void rawLine(const std::string& text);
    void divider();
    void blank();
    std::string readLine(const std::string& promptText);

private:
    void emitColored(Voice voice, const std::string& body);
    void typewriter(const std::string& body, int msPerChar);
    int delayFor(Voice voice) const;

    Config config_;
    bool ansiActive_;
};

}

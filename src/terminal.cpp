#include "terminal.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include "util/ansi.hpp"

#if defined(_WIN32)
#include <conio.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace vestige {

namespace {

std::string_view colorFor(Voice voice) {
    switch (voice) {
        case Voice::Room: return ansi::WHITE;
        case Voice::Log: return ansi::YELLOW;
        case Voice::Aeon: return ansi::CYAN;
        case Voice::Rewritten: return ansi::MAGENTA;
        case Voice::Error: return ansi::RED;
        case Voice::System: return ansi::BRIGHT_WHITE;
    }
    return ansi::WHITE;
}

bool dimmed(Voice voice) {
    return voice == Voice::Log;
}

std::string prefixFor(Voice voice) {
    switch (voice) {
        case Voice::Room: return "";
        case Voice::Log: return "[ARCHIVE] ";
        case Voice::Aeon: return "AEON :: ";
        case Voice::Rewritten: return "[REVISED] ";
        case Voice::Error: return "[FAULT] ";
        case Voice::System: return ">> ";
    }
    return "";
}

bool skipRequested() {
#if defined(_WIN32)
    if (_kbhit() != 0) {
        const int key = _getch();
        return key == '\r' || key == '\n';
    }
    return false;
#else
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    char c = 0;
    const ssize_t got = read(STDIN_FILENO, &c, 1);
    fcntl(STDIN_FILENO, F_SETFL, flags);
    return got > 0 && (c == '\n' || c == '\r');
#endif
}

}

Terminal::Terminal(const Config& config, bool ansiActive)
    : config_(config), ansiActive_(ansiActive) {
    if (ansiActive_) {
        std::cout << ansi::HIDE_CURSOR;
        std::cout.flush();
    }
}

Terminal::~Terminal() {
    if (ansiActive_) {
        std::cout << ansi::SHOW_CURSOR << ansi::RESET;
        std::cout.flush();
    }
}

int Terminal::delayFor(Voice voice) const {
    if (voice == Voice::Aeon || voice == Voice::Rewritten) {
        return config_.aeonMsPerChar;
    }
    return config_.typewriterMsPerChar;
}

void Terminal::typewriter(const std::string& body, int msPerChar) {
    bool skip = msPerChar <= 0;
    for (std::size_t i = 0; i < body.size(); ++i) {
        std::cout << body[i];
        std::cout.flush();
        if (!skip) {
            if (skipRequested()) {
                skip = true;
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(msPerChar));
        }
    }
}

void Terminal::emitColored(Voice voice, const std::string& body) {
    if (ansiActive_) {
        if (dimmed(voice)) {
            std::cout << ansi::DIM;
        }
        std::cout << colorFor(voice);
    }
    typewriter(body, delayFor(voice));
    if (ansiActive_) {
        std::cout << ansi::RESET;
    }
    std::cout << '\n';
    std::cout.flush();
}

void Terminal::say(Voice voice, const std::string& text) {
    emitColored(voice, prefixFor(voice) + text);
}

void Terminal::rawLine(const std::string& text) {
    std::cout << text << '\n';
    std::cout.flush();
}

void Terminal::divider() {
    std::string line(static_cast<std::size_t>(config_.dividerWidth), ansi::DIVIDER_CHAR);
    if (ansiActive_) {
        std::cout << ansi::DIM;
    }
    std::cout << line;
    if (ansiActive_) {
        std::cout << ansi::RESET;
    }
    std::cout << '\n';
    std::cout.flush();
}

void Terminal::blank() {
    std::cout << '\n';
    std::cout.flush();
}

std::string Terminal::readLine(const std::string& promptText) {
    if (ansiActive_) {
        std::cout << ansi::SHOW_CURSOR << ansi::BRIGHT_WHITE;
    }
    std::cout << promptText;
    if (ansiActive_) {
        std::cout << ansi::RESET;
    }
    std::cout.flush();
    std::string input;
    if (!std::getline(std::cin, input)) {
        return "quit";
    }
    if (ansiActive_) {
        std::cout << ansi::HIDE_CURSOR;
        std::cout.flush();
    }
    return input;
}

}

#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace vestige::strutil {

inline std::string toLower(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

inline std::string trim(std::string_view text) {
    const auto notSpace = [](unsigned char c) { return std::isspace(c) == 0; };
    auto begin = std::find_if(text.begin(), text.end(), notSpace);
    auto end = std::find_if(text.rbegin(), text.rend(), notSpace).base();
    if (begin >= end) {
        return std::string{};
    }
    return std::string{begin, end};
}

inline std::vector<std::string> split(std::string_view text, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : text) {
        if (c == delimiter) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

inline std::vector<std::string> words(std::string_view text) {
    std::vector<std::string> result;
    std::string current;
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c)) != 0) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        result.push_back(current);
    }
    return result;
}

inline bool partialMatch(std::string_view candidate, std::string_view query) {
    if (query.empty()) {
        return false;
    }
    const std::string loweredCandidate = toLower(candidate);
    const std::string loweredQuery = toLower(query);
    return loweredCandidate.find(loweredQuery) != std::string::npos;
}

inline std::string collapseWhitespace(std::string_view text) {
    std::string out;
    bool previousSpace = false;
    for (char c : text) {
        const bool isSpace = std::isspace(static_cast<unsigned char>(c)) != 0;
        if (isSpace) {
            if (!previousSpace && !out.empty()) {
                out.push_back(' ');
            }
            previousSpace = true;
        } else {
            out.push_back(c);
            previousSpace = false;
        }
    }
    return trim(out);
}

}

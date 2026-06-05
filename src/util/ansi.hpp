#pragma once

#include <string_view>

namespace vestige::ansi {

inline constexpr std::string_view RESET = "\x1b[0m";
inline constexpr std::string_view BOLD = "\x1b[1m";
inline constexpr std::string_view DIM = "\x1b[2m";

inline constexpr std::string_view WHITE = "\x1b[37m";
inline constexpr std::string_view BRIGHT_WHITE = "\x1b[97m";
inline constexpr std::string_view YELLOW = "\x1b[33m";
inline constexpr std::string_view CYAN = "\x1b[36m";
inline constexpr std::string_view MAGENTA = "\x1b[35m";
inline constexpr std::string_view RED = "\x1b[31m";

inline constexpr std::string_view HIDE_CURSOR = "\x1b[?25l";
inline constexpr std::string_view SHOW_CURSOR = "\x1b[?25h";

inline constexpr char DIVIDER_CHAR = '-';

bool enableVirtualTerminal();

}

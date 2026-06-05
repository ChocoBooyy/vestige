#include "util/ansi.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace vestige::ansi {

bool enableVirtualTerminal() {
#if defined(_WIN32)
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
        return false;
    }
    DWORD mode = 0;
    if (GetConsoleMode(handle, &mode) == 0) {
        return false;
    }
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (SetConsoleMode(handle, mode) == 0) {
        return false;
    }
    SetConsoleOutputCP(CP_UTF8);
    return true;
#else
    return true;
#endif
}

}

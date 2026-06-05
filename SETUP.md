# Vestige: Rider + vcpkg Setup

This document gets the project building in JetBrains Rider on Windows before any game
code is touched. Follow it top to bottom. Gotchas are called out inline.

## 1. Install vcpkg

```bat
git clone https://github.com/microsoft/vcpkg C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat
```

Pick a path with no spaces. `C:\dev\vcpkg` is fine. The bootstrap step builds
`vcpkg.exe`. You do not need `vcpkg integrate install`; this project points CMake at the
vcpkg toolchain file directly, which is cleaner for a single project.

## 2. Install the one dependency

The game parses JSON with nlohmann-json. Install it for the 64 bit triplet:

```bat
C:\dev\vcpkg\vcpkg.exe install nlohmann-json:x64-windows
```

Gotcha: the triplet matters. `nlohmann-json` (no triplet) defaults to `x86-windows` on
some machines, and a 32 bit package will not be found by a 64 bit CMake build. Always
write `:x64-windows`. nlohmann-json is header only, so this step only copies headers and
finishes quickly.

## 3. CMakeLists.txt

The project ships a working `CMakeLists.txt`. The parts that matter for vcpkg are:

```cmake
cmake_minimum_required(VERSION 3.21)
project(Vestige LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(vestige ...)               # explicit source list, no globbing
target_include_directories(vestige PRIVATE src)
target_link_libraries(vestige PRIVATE nlohmann_json::nlohmann_json)
```

`find_package(... CONFIG REQUIRED)` resolves because the vcpkg toolchain file adds the
installed package to the CMake search path. You do not hardcode include directories.

## 4. Point Rider at the vcpkg toolchain

Open the project folder in Rider. Go to `Settings > Build, Execution, Deployment > CMake`.
You will configure one profile.

- Profile name: `Debug`
- Build type: `Debug`
- Toolchain: either `Visual Studio` (MSVC) or `MinGW`. Both compile this project. MSVC is
  the smoother default on Windows because the `x64-windows` triplet is built with MSVC.
- CMake options, add this line:

```
-DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Use forward slashes in that path even on Windows; CMake prefers them and backslashes can be
eaten as escape characters.

Gotcha: if you chose the MinGW toolchain, also add `-DVCPKG_TARGET_TRIPLET=x64-windows` so
vcpkg does not switch to a MinGW triplet that has no package installed. With the MSVC
toolchain the default triplet already matches.

Apply. Rider reloads CMake. The output pane should end with `Configuring done` and
`Generating done`. If it cannot find `nlohmann_json`, the toolchain line is wrong or the
package was installed for the wrong triplet; recheck steps 2 and 4.

## 5. Smoke test before any game code

Before trusting the toolchain, build and run one line. Drop this in `src/main.cpp`
temporarily:

```cpp
#include <cstdio>

int main() {
    std::printf("\x1b[36mVestige toolchain is alive.\x1b[0m\n");
    return 0;
}
```

Build (`Ctrl+F9`) and run (`Shift+F10`). Exit code 0 and one cyan line means the compiler,
the C++23 standard setting, and the run configuration all work. If the line prints with raw
`<-[36m` garbage instead of color, that is the ANSI gotcha in step 6, not a build failure.

Once green, replace `main.cpp` with the real source and rebuild.

## 6. Windows ANSI gotcha

Older Windows consoles do not interpret ANSI escape codes by default, so colors and the
typewriter formatting come out as literal escape sequences. The game handles this itself: at
startup it calls `GetStdHandle` and sets `ENABLE_VIRTUAL_TERMINAL_PROCESSING` through
`SetConsoleMode` (see `src/util/ansi.cpp`). If that call fails, the game falls back to plain
text instead of printing garbage.

You do not need to configure anything for this. It is noted here so that if you see escape
codes during the step 5 smoke test (which does not enable virtual terminal processing), you
know the real game will not have that problem. Windows Terminal and modern PowerShell enable
it anyway; the old `cmd.exe` console host is the one that needs the WinAPI call.

## 7. Run from the project root

The game reads its content from `resources/` relative to the working directory. The build
also copies `resources/` next to the executable as a post build step, so running from either
the project root or the build output folder works. If you ever see
`Archive content failed to load`, your working directory has no `resources/` folder beside
it; set the run configuration working directory to the project root.

## Summary checklist

- vcpkg cloned and bootstrapped at a space free path
- `nlohmann-json:x64-windows` installed
- Rider CMake profile has `-DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake`
- MinGW users also set `-DVCPKG_TARGET_TRIPLET=x64-windows`
- smoke test prints one cyan line and exits 0
- run configuration working directory contains `resources/`

cmake_minimum_required(VERSION 3.21)

project(Vestige LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(vestige
    src/main.cpp
    src/app.cpp
    src/engine.cpp
    src/terminal.cpp
    src/parser.cpp
    src/game_state.cpp
    src/endings.cpp
    src/config/config.cpp
    src/util/ansi.cpp
    src/world/world_graph.cpp
    src/puzzle/puzzle.cpp
    src/puzzle/cipher_puzzle.cpp
    src/puzzle/sequence_puzzle.cpp
    src/puzzle/logic_puzzle.cpp
    src/puzzle/fragment_puzzle.cpp
    src/puzzle/redaction_puzzle.cpp
    src/puzzle/puzzle_registry.cpp
    src/aeon/aeon.cpp
    src/io/save_manager.cpp
    src/io/content_loader.cpp
)

target_include_directories(vestige PRIVATE src)
target_compile_features(vestige PRIVATE cxx_std_23)
target_link_libraries(vestige PRIVATE nlohmann_json::nlohmann_json)

if(MSVC)
    target_compile_options(vestige PRIVATE /W4 /utf-8)
else()
    target_compile_options(vestige PRIVATE -Wall -Wextra)
endif()

add_custom_command(TARGET vestige POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:vestige>/resources
    COMMENT "copy archive content next to the executable")

# Vestige

A terminal text adventure in C++23. You are a memory fragment inside a dying archival
system built to preserve the last records of a civilization that chose to erase itself.
AEON, the archival intelligence, has been alone so long it began rewriting the records to
cope with what they say. You move through the archive's sectors, read logs, repair corrupted
records, and find that some repairs have two valid endings: one that accepts AEON's revised
history, one that restores the original. AEON tracks which version you side with. The ending
reflects what you chose to believe and what you did with it.

No external assets. All output is text, rendered with ANSI escape codes, a typewriter effect,
and a little ASCII framing. It runs in any terminal.

## Build

Prerequisites: CMake 3.21+, a C++23 compiler (MSVC or GCC), and vcpkg with
`nlohmann-json:x64-windows` installed. New to the toolchain on Windows with Rider? Read
`SETUP.md` first.

```sh
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

The build copies `resources/` next to the executable, so you can run it from the build
folder or from the project root. The game looks for `resources/` in the current working
directory.

```sh
./build/vestige        # or build\Debug\vestige.exe depending on the generator
```

If you see `Archive content failed to load`, run it from a directory that has `resources/`
beside it.

## Controls

Type commands at the `>` prompt. Partial words and common synonyms are accepted (`exa`,
`look`, `n` for north, and so on). During the typewriter effect, press Enter to skip to the
end of the current line.

| Command | What it does |
| --- | --- |
| `go [direction]` | Move between sectors. Bare directions work too (`north`, `n`). |
| `examine [target]` | Inspect the room, an item, or a record. No target re-describes the room. |
| `take [item]` | Pick up an item. You can carry eight. |
| `use [item] on [target]` | Apply a carried item. Some open exits, some surface records. |
| `read [record]` | Read a record. If it is corrupted, this starts its repair. |
| `inventory` | List what you carry. |
| `map` | Diagram of the sectors you have discovered. |
| `codex` | List the records you have recovered and their state. |
| `status` | Show progress and AEON's current disposition. |
| `save` / `load` | Store or restore a snapshot. |
| `quit` | Suspend the session. |

Repairing a corrupted record means solving a small puzzle: a substitution cipher, a
reordered boot sequence, a faulted boolean expression, a fragment that does not belong, or a
redacted word you infer from a second record. When a record has two reconstructions, you
choose which one to commit. That choice is the heart of the game.

## Architecture

Everything renders through a single `Terminal` class with five voices (room, log, AEON,
revised, fault, system). Nothing else writes to standard output. Content lives in JSON under
`resources/` and is loaded into typed structs at startup; a missing or malformed file logs a
fault and exits without a stack trace reaching the player.

```
src/
  main.cpp              entry point
  app.*                 init and teardown, owns the Engine
  engine.*              main loop and command dispatch, owns every subsystem
  terminal.*            the only thing that prints, typewriter and voices
  parser.*              raw input to a Command struct
  command.hpp           Command: verb, noun, target, raw
  game_state.*          inventory, discovery, puzzle and flag state
  endings.*             scoring function over the play state, five endings
  world/                rooms, exits, items, logs, the world graph
  puzzle/               sealed Puzzle base and the five concrete types, registry
  aeon/                 trust and awareness tracking, unprompted line selection
  io/                   content loader and save manager
  config/               typed wrapper over config.json
  util/                 string helpers and ANSI constants plus the Windows VT enable
resources/
  rooms.json items.json puzzles.json logs.json aeon_lines.json endings.json config.json
```

AEON keeps two hidden values, trust and awareness, that no screen ever shows. Accepting a
revision raises trust; restoring an original raises awareness. Exits, records, and AEON's
unprompted lines respond to those values, and the final scoring function reads them to pick
one of five endings: Archivist, Collaborator, Agnostic, Witness, Dissolution.

Tuning lives in `resources/config.json`: typewriter speed, the inventory cap, and the
thresholds that decide endings.

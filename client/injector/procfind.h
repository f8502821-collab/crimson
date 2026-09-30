#pragma once
// Finds the running Minecraft (javaw/java) process.
#include <cstdint>
#include <string>
#include <vector>

namespace procfind {

struct Proc {
    uint32_t   pid = 0;
    std::wstring exe_name;     // e.g. javaw.exe
    std::wstring window_title; // empty if none
    bool       has_window = false;
};

// Scan top-level windows for a title containing "Minecraft"; fallback lists all
// java processes. Sorted: windowed first, then by PID.
std::vector<Proc> find_game_processes();

} // namespace procfind

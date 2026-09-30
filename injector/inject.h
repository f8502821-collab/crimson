#pragma once
// Remote DLL injection: OpenProcess -> VirtualAllocEx -> WriteProcessMemory
// -> CreateRemoteThread(LoadLibraryW) -> verify module present.
#include <string>
#include <vector>

namespace inject {

struct Result {
    bool ok = false;
    std::wstring error;          // empty when ok
    double   elapsed_sec = 0.0;  // total wall time
};

// Progress callback: step index 0..3, step name, 0..1 within step.
using ProgressFn = void(*)(int step, const char* step_name, float frac, void* user);

struct Options {
    uint32_t pid = 0;
    std::wstring dll_path;       // absolute path to crimson_client.dll
};

// Steps: 0 allocate, 1 write, 2 launch, 3 verify.
Result run(const Options& opt, ProgressFn cb = nullptr, void* user = nullptr);

// True if the dll file name is already loaded in the process (uses EnumProcessModules).
bool is_loaded(uint32_t pid, const std::wstring& dll_path);

} // namespace inject

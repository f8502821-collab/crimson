#include "injector/inject.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <chrono>
#include <thread>

namespace inject {

static const char* TAG = "inject";

Result run(const Options& opt, ProgressFn cb, void* user) {
    Result res;
    const auto t0 = std::chrono::steady_clock::now();
    auto snap = [&](float f) {
        if (res.elapsed_sec == 0.0) {
            res.elapsed_sec = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - t0).count();
        }
        (void)f;
    };

    // step 0: open + allocate
    if (cb) cb(0, "allocating", 0.f, user);
    HANDLE h = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                               PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                           FALSE, opt.pid);
    if (!h) {
        res.error = L"OpenProcess failed";
        clog::error(TAG, "OpenProcess(pid=%u) failed gle=%lu", opt.pid, GetLastError());
        return res;
    }
    const size_t path_bytes = (opt.dll_path.size() + 1) * sizeof(wchar_t);
    void* remote = VirtualAllocEx(h, nullptr, path_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) {
        res.error = L"VirtualAllocEx failed";
        clog::error(TAG, "VirtualAllocEx failed gle=%lu", GetLastError());
        CloseHandle(h);
        return res;
    }
    if (cb) cb(0, "allocating", 1.f, user);

    // step 1: write path
    if (cb) cb(1, "writing", 0.f, user);
    SIZE_T written = 0;
    if (!WriteProcessMemory(h, remote, opt.dll_path.c_str(), path_bytes, &written) ||
        written != path_bytes) {
        res.error = L"WriteProcessMemory failed";
        clog::error(TAG, "WriteProcessMemory failed gle=%lu", GetLastError());
        VirtualFreeEx(h, remote, 0, MEM_RELEASE);
        CloseHandle(h);
        return res;
    }
    if (cb) cb(1, "writing", 1.f, user);

    // step 2: remote thread on LoadLibraryW
    if (cb) cb(2, "launching", 0.f, user);
    HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
    auto load_lib = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(k32, "LoadLibraryW"));
    if (!load_lib) {
        res.error = L"GetProcAddress(LoadLibraryW) failed";
        CloseHandle(h);
        return res;
    }
    HANDLE th = CreateRemoteThread(h, nullptr, 0, load_lib, remote, 0, nullptr);
    if (!th) {
        res.error = L"CreateRemoteThread failed";
        clog::error(TAG, "CreateRemoteThread failed gle=%lu", GetLastError());
        VirtualFreeEx(h, remote, 0, MEM_RELEASE);
        CloseHandle(h);
        return res;
    }
    WaitForSingleObject(th, 8000);
    if (cb) cb(2, "launching", 1.f, user);

    // step 3: verify module present
    if (cb) cb(3, "verifying", 0.f, user);
    const wchar_t* fname = opt.dll_path.c_str();
    const wchar_t* base = fname;
    for (const wchar_t* p = fname; *p; ++p)
        if (*p == L'\\' || *p == L'/') base = p + 1;

    bool loaded = false;
    for (int attempt = 0; attempt < 10 && !loaded; ++attempt) {
        HMODULE mods[1024];
        DWORD needed = 0;
        if (EnumProcessModules(h, mods, sizeof(mods), &needed)) {
            const int count = (int)(needed / sizeof(HMODULE));
            const int capped = count > 1024 ? 1024 : count;
            for (int i = 0; i < capped; ++i) {
                wchar_t name[MAX_PATH];
                if (GetModuleFileNameExW(h, mods[i], name, MAX_PATH)) {
                    const wchar_t* nb = name;
                    for (const wchar_t* p = name; *p; ++p)
                        if (*p == L'\\' || *p == L'/') nb = p + 1;
                    if (_wcsicmp(nb, base) == 0) { loaded = true; break; }
                }
            }
        }
        if (!loaded) std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    CloseHandle(th);
    VirtualFreeEx(h, remote, 0, MEM_RELEASE);
    CloseHandle(h);

    res.ok = loaded;
    if (!loaded) res.error = L"DLL did not appear in module list (load failed inside target)";
    if (cb) cb(3, "verifying", 1.f, user);
    snap(1.f);
    clog::info(TAG, "inject pid=%u ok=%d %.2fs", opt.pid, res.ok ? 1 : 0, res.elapsed_sec);
    return res;
}

bool is_loaded(uint32_t pid, const std::wstring& dll_path) {
    HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!h) return false;
    const wchar_t* fname = dll_path.c_str();
    const wchar_t* base = fname;
    for (const wchar_t* p = fname; *p; ++p)
        if (*p == L'\\' || *p == L'/') base = p + 1;

    bool found = false;
    HMODULE mods[1024];
    DWORD needed = 0;
    if (EnumProcessModules(h, mods, sizeof(mods), &needed)) {
        const int count = (int)(needed / sizeof(HMODULE));
        const int capped = count > 1024 ? 1024 : count;
        for (int i = 0; i < capped && !found; ++i) {
            wchar_t name[MAX_PATH];
            if (GetModuleFileNameExW(h, mods[i], name, MAX_PATH)) {
                const wchar_t* nb = name;
                for (const wchar_t* p = name; *p; ++p)
                    if (*p == L'\\' || *p == L'/') nb = p + 1;
                if (_wcsicmp(nb, base) == 0) found = true;
            }
        }
    }
    CloseHandle(h);
    return found;
}

} // namespace inject

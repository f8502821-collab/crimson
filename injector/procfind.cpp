#include "injector/procfind.h"
#include <windows.h>
#include <tlhelp32.h>
#include <algorithm>

namespace procfind {

struct EnumCtx {
    std::vector<Proc>* out;
};

static BOOL CALLBACK on_window(HWND hwnd, LPARAM lp) {
    auto* ctx = reinterpret_cast<EnumCtx*>(lp);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (!pid) return TRUE;

    wchar_t title[256]{};
    GetWindowTextW(hwnd, title, 256);
    if (!title[0]) return TRUE;
    if (!IsWindowVisible(hwnd)) return TRUE;

    // match any visible window whose title mentions Minecraft
    std::wstring t(title);
    if (t.find(L"Minecraft") == std::wstring::npos &&
        t.find(L"minecraft") == std::wstring::npos) {
        return TRUE;
    }

    Proc p;
    p.pid = pid;
    p.window_title = t;
    p.has_window = true;

    // exe name via snapshot
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(snap, &pe)) {
            do {
                if (pe.th32ProcessID == pid) {
                    p.exe_name = pe.szExeFile;
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }
    if (p.exe_name.empty()) p.exe_name = L"?";

    // merge into existing entry if the pid already appeared
    for (auto& e : *ctx->out)
        if (e.pid == pid) { e.has_window = true; e.window_title = t; return TRUE; }
    ctx->out->push_back(p);
    return TRUE;
}

std::vector<Proc> find_game_processes() {
    std::vector<Proc> out;
    EnumCtx ctx{ &out };
    EnumWindows(on_window, reinterpret_cast<LPARAM>(&ctx));

    // fallback: any javaw/java process not already listed
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(snap, &pe)) {
            do {
                const std::wstring exe = pe.szExeFile;
                const bool is_java =
                    _wcsicmp(exe.c_str(), L"javaw.exe") == 0 ||
                    _wcsicmp(exe.c_str(), L"java.exe") == 0;
                if (!is_java) continue;
                bool listed = false;
                for (auto& e : out) if (e.pid == pe.th32ProcessID) { listed = true; break; }
                if (listed) continue;
                Proc p;
                p.pid = pe.th32ProcessID;
                p.exe_name = exe;
                out.push_back(p);
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }

    std::sort(out.begin(), out.end(), [](const Proc& a, const Proc& b) {
        if (a.has_window != b.has_window) return a.has_window;
        return a.pid < b.pid;
    });
    return out;
}

} // namespace procfind

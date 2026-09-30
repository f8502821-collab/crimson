// crimson_client.dll entry point.
// Rule #1 of injected DLLs: DllMain does the absolute minimum - the loader
// thread does everything else, or we risk loader-lock deadlocks.
#include "client/loader.h"
#include <windows.h>

static HANDLE g_thread = nullptr;

static DWORD WINAPI load_thread(LPVOID) {
    client::loader::load();
    return 0;
}

static DWORD WINAPI unload_thread(LPVOID) {
    client::loader::unload(false);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(mod);
        client::set_self_module((void*)mod);
        g_thread = CreateThread(nullptr, 0, load_thread, nullptr, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        // best-effort synchronous teardown; the normal path is unload(true)
        client::loader::shutdown_static();
        break;
    default:
        break;
    }
    return TRUE;
}

namespace client {
void request_uninject_async() {
    if (g_thread) { CloseHandle(g_thread); g_thread = nullptr; }
    CreateThread(nullptr, 0, unload_thread, nullptr, 0, nullptr);
}
} // namespace client

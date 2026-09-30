#include "client/loader.h"
#include "client/hooks/hooks.h"
#include "client/jvm/jvm.h"
#include "client/jvm/mappings.h"
#include "client/jvm/mc.h"
#include "client/modules/module_manager.h"
#include "client/util/config.h"
#include "client/render/renderer.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

bool client::LoadState::jvm_ok = false;
bool client::LoadState::hooks_ok = false;
bool client::LoadState::modules_ok = false;
bool client::LoadState::gui_ready = false;

namespace client {

static const char* TAG = "loader";
static HMODULE g_self = nullptr;

void set_self_module(void* m) { g_self = (HMODULE)m; }

static std::string log_path() {
    char buf[MAX_PATH]{};
    GetTempPathA(MAX_PATH, buf);
    return std::string(buf) + "crimson_client.log";
}

void load() {
    clog::init(log_path().c_str());
    clog::info(TAG, "=== crimson_client load ===");

    // 1. attach to the JVM first: everything else needs a JNIEnv
    LoadState::jvm_ok = jvm::init();
    if (!LoadState::jvm_ok) {
        clog::error(TAG, "jvm attach failed - unloading");
        unload(true);
        return;
    }

    // 2. resolve every mapping once; failures disable only affected modules
    mappings::resolve_all();

    // 3. static state
    LoadState::modules_ok = modules::init_all();
    config::load();

    // 4. hooks last: after this, frames arrive
    LoadState::hooks_ok = hooks::install();
    if (!LoadState::hooks_ok) {
        clog::error(TAG, "hook install failed - unloading");
        unload(true);
        return;
    }

    // 5. renderer registers the per-frame callback
    renderer::init();
    client::register_frame_entry();
    LoadState::gui_ready = true;
    clog::info(TAG, "load complete");
}

void unload(bool free_library) {
    clog::info(TAG, "unload (free=%d)", free_library ? 1 : 0);
    hooks::uninstall();          // stop frame flow first
    modules::shutdown_all();
    config::save();
    jvm::shutdown();
    clog::info(TAG, "unload complete");
    clog::shutdown();
    if (free_library && g_self) {
        HMODULE self = g_self;
        // FreeLibraryAndExitThread on a spawned thread so we're not freeing
        // ourselves from inside our own call stack.
        FreeLibraryAndExitThread(self, 0);
    }
}

void shutdown_static() {
    // best-effort when the host tears the DLL down itself
    hooks::uninstall();
    clog::shutdown();
}

double now_sec() {
    static const long long freq_qpc = [] {
        LARGE_INTEGER f; QueryPerformanceFrequency(&f); return f.QuadPart;
    }();
    LARGE_INTEGER c; QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)freq_qpc;
}

void register_frame_entry();  // implemented in renderer.cpp

} // namespace client

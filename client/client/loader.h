#pragma once
// Lifecycle: jvm attach -> mapping resolve -> hooks -> modules -> shutdown.
#include <cstdint>

namespace client {

// Status flags the GUI can surface.
struct LoadState {
    static bool jvm_ok;
    static bool hooks_ok;
    static bool modules_ok;
    static bool gui_ready;
};

void load();                       // runs on the loader thread
void unload(bool free_library);    // INSERT->END panic or clean unload
void shutdown_static();            // best-effort on DLL_PROCESS_DETACH
void set_self_module(void* mod);   // called once from DllMain (HMODULE)

void request_uninject_async();     // implemented in dllmain.cpp

// current frame time (seconds, steady clock) - filled by renderer
double now_sec();

} // namespace client

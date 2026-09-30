#include "client/render/renderer.h"
#include "client/render/postfx.h"
#include "client/hooks/hooks.h"
#include "client/gui/theme.h"
#include "client/gui/clickgui.h"
#include "client/gui/hud.h"
#include "client/modules/module_manager.h"
#include "client/jvm/mc.h"
#include "client/loader.h"
#include "common/fonts.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <mutex>
#include <string>
#include <cmath>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace renderer {

static const char* TAG = "renderer";
static bool g_ready = false;
static double g_time = 0;
static mc::WorldSnap g_world;
static mc::PlayerSnap g_player;
static std::mutex g_snap_mtx;

// forward decl for the client::register_frame_entry bridge at file end
void renderer_frame();

// frame entry, invoked from the swap hook on the game's render thread
void renderer_frame() {
    if (!g_ready) {
        if (!init()) return;
    }
    g_time = client::now_sec();

    refresh_snapshots();

    // 1. module updates (movement ticks etc.) before drawing
    {
        std::lock_guard<std::mutex> g(g_snap_mtx);
        modules::manager().update(g_player, g_world, g_time);
    }

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // 2. post-FX before GUI so overlays read on top
    if (clickgui::is_open()) {
        ImGuiIO& io = ImGui::GetIO();
        postfx::apply(g_time, clickgui::fx_amount(), (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    } else {
        const double since = clickgui::time_since_close();
        if (since < 0.5)
            postfx::apply(g_time, clickgui::fx_amount() * (float)(1.0 - since * 2.0), (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    }

    // 3. ESP/overlays draw under the panels, above the FX
    {
        std::lock_guard<std::mutex> g(g_snap_mtx);
        modules::manager().render(g_world, g_player, g_time);
    }

    // 4. GUI + HUD
    clickgui::draw(g_time);
    hud::draw(g_time);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ---- input bridging --------------------------------------------------------

static void on_key(const hooks::KeyEvent& ev) {
    if (ev.vk == VK_INSERT && ev.down && !ev.prev_down) {
        clickgui::toggle();
        return;
    }
    if (ev.vk == VK_END && ev.down && !ev.prev_down) {
        request_uninject_async();
        return;
    }
    if (clickgui::is_open()) {
        ImGuiIO& io = ImGui::GetIO();
        if (ev.vk < 256) {
            io.AddKeyEvent(ImGui::GetKeyIndex((ImGuiKey)ev.vk), ev.down);
        }
    }
    modules::manager().dispatch_key(ev.vk, ev.down);
}

static void on_mouse(int button, bool down) {
    if (clickgui::is_open()) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, down);
        // don't forward to modules while GUI consumes the click
        return;
    }
    modules::manager().dispatch_mouse(button, down);
}

static void on_mouse_move(int x, int y) {
    if (clickgui::is_open()) {
        ImGui::GetIO().AddMousePosEvent((float)x, (float)y);
    }
}

static void on_char(unsigned int ch) {
    if (clickgui::is_open()) {
        ImGui::GetIO().AddInputCharacter(ch);
    }
}

// ---- world snapshot --------------------------------------------------------

static void refresh_snapshots() {
    std::lock_guard<std::mutex> g(g_snap_mtx);
    mc::read_player(g_player);
    mc::read_world(g_world);
}

bool init() {
    if (g_ready) return true;
    // ImGui on the game's GL context, from inside the swap hook (safe point)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    fonts::build();

    // win32 backend bound to the game's window (we still feed events ourselves)
    if (!ImGui_ImplWin32_Init(hooks::current_window())) {
        clog::error(TAG, "win32 backend init failed");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        clog::error(TAG, "gl backend init failed");
        return false;
    }

    postfx::init();

    hooks::set_key_callback(on_key);
    hooks::set_mouse_callback(on_mouse);
    hooks::set_mouse_move_callback(on_mouse_move);
    hooks::set_char_callback(on_char);

    clickgui::init();
    hud::init();
    g_ready = true;
    clog::info(TAG, "renderer ready");
    return true;
}

void shutdown() {
    if (!g_ready) return;
    clickgui::shutdown();
    hud::shutdown();
    postfx::shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_ready = false;
}

} // namespace renderer

namespace client {
void register_frame_entry() {
    hooks::set_frame_callback(&renderer_frame);
}
} // namespace client

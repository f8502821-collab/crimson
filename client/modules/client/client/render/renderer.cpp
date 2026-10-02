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

// forward decls for the client::register_frame_entry bridge at file end
void renderer_frame();
static void refresh_snapshots();

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

// Win32 VK -> ImGuiKey for the keys the GUI cares about (text input arrives
// separately via WM_CHAR). Raw VK values are NOT valid ImGuiKey values.
static ImGuiKey vk_to_imgui(int vk) {
    if (vk >= 'A' && vk <= 'Z') return (ImGuiKey)(ImGuiKey_A + (vk - 'A'));
    if (vk >= '0' && vk <= '9') return (ImGuiKey)(ImGuiKey_0 + (vk - '0'));
    switch (vk) {
    case VK_TAB:      return ImGuiKey_Tab;
    case VK_LEFT:     return ImGuiKey_LeftArrow;
    case VK_RIGHT:    return ImGuiKey_RightArrow;
    case VK_UP:       return ImGuiKey_UpArrow;
    case VK_DOWN:     return ImGuiKey_DownArrow;
    case VK_PRIOR:    return ImGuiKey_PageUp;
    case VK_NEXT:     return ImGuiKey_PageDown;
    case VK_HOME:     return ImGuiKey_Home;
    case VK_END:      return ImGuiKey_End;
    case VK_INSERT:   return ImGuiKey_Insert;
    case VK_DELETE:   return ImGuiKey_Delete;
    case VK_BACK:     return ImGuiKey_Backspace;
    case VK_SPACE:    return ImGuiKey_Space;
    case VK_RETURN:   return ImGuiKey_Enter;
    case VK_ESCAPE:   return ImGuiKey_Escape;
    case VK_OEM_1:    return ImGuiKey_Semicolon;
    case VK_OEM_PLUS: return ImGuiKey_Equal;
    case VK_OEM_COMMA:return ImGuiKey_Comma;
    case VK_OEM_MINUS:return ImGuiKey_Minus;
    case VK_OEM_PERIOD:return ImGuiKey_Period;
    case VK_OEM_2:    return ImGuiKey_Slash;
    case VK_OEM_3:    return ImGuiKey_GraveAccent;
    case VK_OEM_4:    return ImGuiKey_LeftBracket;
    case VK_OEM_5:    return ImGuiKey_Backslash;
    case VK_OEM_6:    return ImGuiKey_RightBracket;
    case VK_OEM_7:    return ImGuiKey_Apostrophe;
    default:          return ImGuiKey_None;
    }
}

static void on_key(const hooks::KeyEvent& ev) {
    if (ev.vk == VK_INSERT && ev.down && !ev.prev_down) {
        clickgui::toggle();
        return;
    }
    if (ev.vk == VK_END && ev.down && !ev.prev_down) {
        client::request_uninject_async();
        return;
    }
    if (clickgui::is_open()) {
        ImGuiIO& io = ImGui::GetIO();
        const ImGuiKey ik = vk_to_imgui(ev.vk);
        if (ik != ImGuiKey_None) io.AddKeyEvent(ik, ev.down);
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

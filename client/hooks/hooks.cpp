#include "client/hooks/hooks.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <MinHook.h>
#include <GL/gl.h>
#include <cstring>

namespace hooks {

static const char* TAG = "hooks";
static FrameFn g_frame = nullptr;
static void (*g_key)(const KeyEvent&) = nullptr;
static void (*g_mouse)(int, bool) = nullptr;
static void (*g_mouse_move)(int, int) = nullptr;
static void (*g_char)(unsigned int) = nullptr;
static bool g_wants_mouse = false;
static bool g_installed = false;

bool input_wants_mouse() { return g_wants_mouse; }
void set_input_wants_mouse(bool v) { g_wants_mouse = v; }
void set_frame_callback(FrameFn fn) { g_frame = fn; }
void set_key_callback(void(*fn)(const KeyEvent&)) { g_key = fn; }
void set_mouse_callback(void(*fn)(int button, bool down)) { g_mouse = fn; }
void set_mouse_move_callback(void(*fn)(int x, int y)) { g_mouse_move = fn; }
void set_char_callback(void(*fn)(unsigned int ch)) { g_char = fn; }

// ---------------------------------------------------------------------------
// original function pointers
// ---------------------------------------------------------------------------
using swap_buffers_t = BOOL(WINAPI*)(HDC);
static swap_buffers_t g_orig_wgl = nullptr;
using glfw_swap_t = void(APIENTRY*)(void* window);
static glfw_swap_t g_orig_glfw = nullptr;
static void* g_glfw_window = nullptr;

// tracked key state so events carry prev_down
static bool g_keys[256] = {};

static void on_frame() {
    if (g_frame) g_frame();
}

// ---------------------------------------------------------------------------
// wglSwapBuffers hook
// ---------------------------------------------------------------------------
static BOOL WINAPI hk_wglSwapBuffers(HDC hdc) {
    on_frame();
    return g_orig_wgl(hdc);
}

// glfwSwapBuffers fallback (signature: void(void*))
static void APIENTRY hk_glfwSwapBuffers(void* window) {
    if (!g_glfw_window) g_glfw_window = window;
    on_frame();
    g_orig_glfw(g_glfw_window ? g_glfw_window : window);
}

// ---------------------------------------------------------------------------
// WndProc subclass (GLFW creates window class "GLFW30")
// ---------------------------------------------------------------------------
static LONG_PTR g_orig_wndproc = 0;
static WNDPROC g_orig_wndproc_proc = nullptr;
static HWND g_game_window = nullptr;

HWND current_window() { return g_game_window; }
HDC  current_dc()      { return g_game_window ? GetDC(g_game_window) : nullptr; }
HGLRC current_glrc()   { return wglGetCurrentContext(); }

static LRESULT CALLBACK hk_wndproc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        const int vk = (int)wp & 0xFF;
        const bool prev = g_keys[vk];
        g_keys[vk] = true;
        if (g_key) { KeyEvent ev{ vk, true, prev }; g_key(&ev); }
        if (g_wants_mouse && vk != VK_INSERT && vk != VK_END) return 0;
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        const int vk = (int)wp & 0xFF;
        g_keys[vk] = false;
        if (g_key) { KeyEvent ev{ vk, false, true }; g_key(&ev); }
        if (g_wants_mouse) return 0;
        break;
    }
    case WM_LBUTTONDOWN: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: {
        const bool down = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN);
        const int btn = (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? 0
                      : (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) ? 1 : 2;
        if (g_mouse) g_mouse(btn, down);
        if (g_wants_mouse) return 0;
        break;
    }
    case WM_MOUSEMOVE:
        if (g_mouse_move) g_mouse_move((short)LOWORD(lp), (short)HIWORD(lp));
        if (g_wants_mouse) return 0;
        break;
    case WM_MOUSEWHEEL:
        if (g_wants_mouse) return 0;
        break;
    case WM_CHAR:
        if (g_char) g_char((unsigned int)wp);
        if (g_wants_mouse) return 0;
        break;
    case WM_INPUT:
        if (g_wants_mouse) return 0;  // swallow raw input so the camera stops
        break;
    default:
        break;
    }
    return g_orig_wndproc_proc
        ? CallWindowProcW(g_orig_wndproc_proc, h, msg, wp, lp)
        : DefWindowProcW(h, msg, wp, lp);
}

// ---------------------------------------------------------------------------
// install / uninstall
// ---------------------------------------------------------------------------
static bool hook_via_wgl() {
    // grab a real GL context handle from the active window
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) hwnd = FindWindowW(L"GLFW30", nullptr);
    HDC hdc = GetDC(hwnd);
    if (!hdc) return false;
    HGLRC glrc = wglGetCurrentContext();
    if (!glrc) { ReleaseDC(hwnd, hdc); return false; }

    HMODULE opengl32 = GetModuleHandleW(L"opengl32.dll");
    if (!opengl32) { ReleaseDC(hwnd, hdc); return false; }
    void* target = (void*)GetProcAddress(opengl32, "wglSwapBuffers");
    ReleaseDC(hwnd, hdc);
    if (!target) return false;

    if (MH_CreateHook(target, (LPVOID)&hk_wglSwapBuffers, (LPVOID*)&g_orig_wgl) != MH_OK)
        return false;
    if (MH_EnableHook(target) != MH_OK) return false;
    clog::info(TAG, "hooked wglSwapBuffers");
    return true;
}

static bool hook_via_glfw() {
    HMODULE glfw = GetModuleHandleW(L"glfw3.dll");
    if (!glfw) return false;
    void* target = (void*)GetProcAddress(glfw, "glfwSwapBuffers");
    if (!target) return false;
    if (MH_CreateHook(target, (LPVOID)&hk_glfwSwapBuffers, (LPVOID*)&g_orig_glfw) != MH_OK)
        return false;
    if (MH_EnableHook(target) != MH_OK) return false;
    clog::info(TAG, "hooked glfwSwapBuffers (fallback)");
    return true;
}

bool install() {
    if (g_installed) return true;
    if (MH_Initialize() != MH_OK) {
        clog::error(TAG, "MH_Initialize failed");
        return false;
    }
    if (!hook_via_wgl() && !hook_via_glfw()) {
        clog::error(TAG, "no swap hook available (wgl + glfw both failed)");
        MH_Uninitialize();
        return false;
    }

    // wndproc subclass
    HWND hwnd = FindWindowW(L"GLFW30", nullptr);
    g_game_window = hwnd;
    if (hwnd) {
        g_orig_wndproc_proc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
        if (g_orig_wndproc_proc) {
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)&hk_wndproc);
            clog::info(TAG, "wndproc subclassed");
        }
    } else {
        clog::warn(TAG, "GLFW window not found - INSERT/END hotkeys inactive");
    }

    g_installed = true;
    return true;
}

void uninstall() {
    if (!g_installed) return;
    HWND hwnd = FindWindowW(L"GLFW30", nullptr);
    if (hwnd && g_orig_wndproc_proc)
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)g_orig_wndproc_proc);
    g_orig_wndproc_proc = nullptr;
    MH_Uninitialize();
    g_installed = false;
    clog::info(TAG, "hooks removed");
}

} // namespace hooks

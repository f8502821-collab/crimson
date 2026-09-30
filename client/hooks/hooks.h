#pragma once
// Hook installation: wglSwapBuffers (fallback glfwSwapBuffers) + WndProc.
#include <cstdint>

namespace hooks {

using FrameFn = void(*)();                 // called once per rendered frame

bool install();
void uninstall();

bool input_wants_mouse();                  // GUI open?
void set_input_wants_mouse(bool v);        // called by the GUI layer

// forward decls to keep windows.h out of this header
struct HWND__;
typedef HWND__* HWND;
struct HDC__;
typedef HDC__* HDC;
struct HGLRC__;
typedef HGLRC__* HGLRC;

// window/context getters used by the win32 backend + postfx init
HWND current_window();
HDC  current_dc();
HGLRC current_glrc();

void set_frame_callback(FrameFn fn);

// key events the GUI/module layer consumes
struct KeyEvent {
    int    vk = 0;
    bool   down = false;
    bool   prev_down = false;              // state before this event
};

void set_key_callback(void(*fn)(const KeyEvent&));
void set_mouse_callback(void(*fn)(int button, bool down));
void set_mouse_move_callback(void(*fn)(int x, int y));
void set_char_callback(void(*fn)(unsigned int ch));

} // namespace hooks

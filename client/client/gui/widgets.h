#pragma once
// Custom widgets: animated toggle, slider, chips, keybind, collapse.
#include <imgui.h>
#include <cstdint>

namespace widgets {

// sliding pill toggle with glow burst; returns new state
bool toggle(const char* id, bool* v);

// crimson gradient slider
bool slider(const char* id, float* v, float mn, float mx, const char* fmt = "%.1f");

// module chip: name + state dot; clickable; highlights when bound key pressed state
bool chip(const char* label, bool active, bool* toggled);

// keybind button: click then press a key; returns captured vk or 0
bool keybind(const char* id, int* vk);

// animated section header; returns open state
bool section(const char* label, bool* open);

// notification toast store
void toast(const char* msg);
void draw_toasts(double now);

} // namespace widgets

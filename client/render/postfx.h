#pragma once
// Post-FX pass: copy the game frame through a GLSL shader, then ImGui on top.
#include <cstdint>

namespace postfx {

void init();                       // build program + resources (call once, GL ctx current)
void shutdown();

// Call before ImGui renders each frame while the menu is open (or blending).
// time_s = animation clock; amount 0..1 = effect intensity; w/h = backbuffer size
void apply(double time_s, float amount, int w, int h);

// last frame the FX was applied (for fade-out lerp)
double last_apply_time();

} // namespace postfx

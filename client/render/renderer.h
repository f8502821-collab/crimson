#pragma once
// Frame orchestration: NewFrame -> modules -> postfx -> gui -> present hook tail.
#include <cstdint>

namespace renderer {

bool init();        // ImGui context, fonts, postfx (GL ctx current, inside hook)
void shutdown();

} // namespace renderer

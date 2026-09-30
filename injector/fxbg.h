#pragma once
// Animated injector background: drifting crimson waves, aurora blobs,
// particle dust, vignette. Pure DrawList, no textures.
#include <imgui.h>

namespace fxbg {

void draw(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, double time);

} // namespace fxbg

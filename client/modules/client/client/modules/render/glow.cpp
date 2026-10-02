#include "client/modules/render/glow.h"
#include "client/jvm/mc.h"
#include "client/util/projection.h"
#include "common/theme.h"
#include "common/uikit.h"
#include <imgui.h>
#include <cmath>
#include <algorithm>

namespace modules {

void Glow::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    const proj::Vec3 cam{ p.x, p.y + 1.62, p.z };
    float fov = 70.f;
    mc::read_fov(fov);

    for (const auto& e : w.players) {
        float sx, sy;
        if (!proj::world_to_screen({ e.x, e.y + 0.9, e.z }, cam,
                                   p.yaw, p.pitch, fov, W.x, W.y, sx, sy))
            continue;
        // approximate on-screen height from bbox corners
        float hx, hy;
        if (!proj::world_to_screen({ e.x, e.max_y, e.z }, cam,
                                   p.yaw, p.pitch, fov, W.x, W.y, hx, hy))
            continue;
        const float h = std::fabs(hy - sy);
        const float rad = h * 0.8f;
        if (rad < 2.f) continue;
        // soft crimson aura, layered
        for (int i = 5; i >= 1; --i) {
            const float rr = rad * (float)i / 5.f;
            dl->AddCircleFilled(ImVec2(sx, sy), rr,
                                theme::with_alpha(theme::ACCENT, 0.028f));
        }
    }
}

} // namespace modules

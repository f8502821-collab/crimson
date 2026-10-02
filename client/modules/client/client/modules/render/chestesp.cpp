#include "client/modules/render/chestesp.h"
#include "client/jvm/mc.h"
#include "client/util/projection.h"
#include "common/theme.h"
#include "common/uikit.h"
#include <imgui.h>
#include <cmath>

namespace modules {

void ChestEsp::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    const proj::Vec3 cam{ p.x, p.y + 1.62, p.z };
    float fov = 70.f;
    mc::read_fov(fov);

    for (const auto& c : w.chests) {
        // chest block box (slightly inset like the real model)
        const proj::Vec3 pts[8] = {
            { c.x + 0.0625, c.y,          c.z + 0.0625 },
            { c.x + 0.9375, c.y,          c.z + 0.0625 },
            { c.x + 0.9375, c.y,          c.z + 0.9375 },
            { c.x + 0.0625, c.y,          c.z + 0.9375 },
            { c.x + 0.0625, c.y + 0.875,  c.z + 0.0625 },
            { c.x + 0.9375, c.y + 0.875,  c.z + 0.0625 },
            { c.x + 0.9375, c.y + 0.875,  c.z + 0.9375 },
            { c.x + 0.0625, c.y + 0.875,  c.z + 0.9375 },
        };
        float sx, sy;
        if (!proj::world_to_screen({ c.x + 0.5, c.y + 0.45, c.z + 0.5 }, cam,
                                   p.yaw, p.pitch, fov, W.x, W.y, sx, sy))
            continue;

        // distance-scaled diamond marker
        const float size = std::fmax(220.f / (float)(c.dist + 1.0), 3.f);
        const ImU32 col = theme::with_alpha(theme::ACCENT_HOT, 0.85f);
        dl->AddLine(ImVec2(sx - size, sy), ImVec2(sx, sy - size * 0.7f), col, 1.3f);
        dl->AddLine(ImVec2(sx, sy - size * 0.7f), ImVec2(sx + size, sy), col, 1.3f);
        dl->AddLine(ImVec2(sx + size, sy), ImVec2(sx, sy + size * 0.7f), col, 1.3f);
        dl->AddLine(ImVec2(sx, sy + size * 0.7f), ImVec2(sx - size, sy), col, 1.3f);
        if (c.dist < 24.0) {
            dl->AddCircleFilled(ImVec2(sx, sy), size * 0.35f,
                                theme::with_alpha(theme::ACCENT, 0.10f));
        }
    }
}

} // namespace modules

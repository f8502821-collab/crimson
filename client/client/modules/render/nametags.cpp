#include "client/modules/render/nametags.h"
#include "client/jvm/mc.h"
#include "client/util/projection.h"
#include "common/theme.h"
#include "common/fonts.h"
#include <imgui.h>
#include <cstdio>
#include <cmath>

namespace modules {

void Nametags::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    const proj::Vec3 cam{ p.x, p.y + 1.62, p.z };
    float fov = 70.f;
    mc::read_fov(fov);

    ImGui::PushFont(fonts::mono());
    for (const auto& e : w.players) {
        if (e.name.empty()) continue;
        proj::Vec3 above{ e.x, e.max_y + 0.35, e.z };
        float sx, sy;
        if (!proj::world_to_screen(above, cam, p.yaw, p.pitch, fov, W.x, W.y, sx, sy))
            continue;

        char text[160];
        snprintf(text, sizeof(text), "%s  %.0f", e.name.c_str(), e.health);
        const ImVec2 ts = ImGui::CalcTextSize(text);
        const ImVec2 mn(sx - ts.x * 0.5f - 6.f, sy - ts.y * 0.5f - 3.f);
        const ImVec2 mx(sx + ts.x * 0.5f + 6.f, sy + ts.y * 0.5f + 3.f);
        dl->AddRectFilled(mn, mx, theme::with_alpha(theme::BG_GLASS_2, 0.78f), 4.f);
        dl->AddRect(mn, mx, theme::with_alpha(theme::ACCENT, 0.4f), 4.f);
        dl->AddText(ImVec2(mn.x + 6.f, mn.y + 3.f), theme::TEXT_HI, text);
    }
    ImGui::PopFont();
}

} // namespace modules

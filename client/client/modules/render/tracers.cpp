#include "client/modules/render/tracers.h"
#include "client/jvm/mc.h"
#include "client/util/projection.h"
#include "common/theme.h"
#include <imgui.h>
#include <cmath>

namespace modules {

void Tracers::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    const proj::Vec3 cam{ p.x, p.y + 1.62, p.z };
    const ImVec2 origin(W.x * 0.5f, W.y);   // bottom center
    float fov = 70.f;
    mc::read_fov(fov);

    for (const auto& e : w.players) {
        proj::Vec3 target{ e.x, e.y + 0.9, e.z };
        float sx, sy;
        if (!proj::world_to_screen(target, cam, p.yaw, p.pitch, fov, W.x, W.y, sx, sy))
            continue;
        const float hpf = std::fmin(std::fmax(e.health / 20.f, 0.f), 1.f);
        const ImU32 col = theme::mix(theme::ACCENT_DEEP, theme::ACCENT, hpf);
        dl->AddLine(origin, ImVec2(sx, sy), theme::with_alpha(col, 0.55f), 1.2f);
        dl->AddCircleFilled(ImVec2(sx, sy), 2.f, theme::with_alpha(col, 0.9f));
    }
}

} // namespace modules

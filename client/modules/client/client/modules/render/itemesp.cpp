#include "client/modules/render/itemesp.h"
#include "client/jvm/mc.h"
#include "client/util/projection.h"
#include "common/theme.h"
#include <imgui.h>
#include <cmath>

namespace modules {

void ItemEsp::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    const proj::Vec3 cam{ p.x, p.y + 1.62, p.z };
    float fov = 70.f;
    mc::read_fov(fov);

    for (const auto& e : w.items) {
        float sx, sy;
        if (!proj::world_to_screen({ e.x, e.y + 0.3, e.z }, cam,
                                   p.yaw, p.pitch, fov, W.x, W.y, sx, sy))
            continue;
        const float bob = sinf((float)now * 3.f + (float)e.x) * 2.f;
        const float r = std::fmax(90.f / (float)(e.dist + 1.0), 2.f);
        dl->AddCircle(ImVec2(sx, sy + bob), r, theme::with_alpha(theme::OK, 0.7f), 0, 1.2f);
        dl->AddCircleFilled(ImVec2(sx, sy + bob), 1.5f, theme::with_alpha(theme::OK, 0.9f));
    }
}

} // namespace modules

#include "client/modules/render/esp.h"
#include "client/jvm/mc.h"
#include "client/util/projection.h"
#include "common/theme.h"
#include "common/uikit.h"
#include <imgui.h>
#include <cmath>
#include <cstdio>

namespace modules {

static bool project_box_corners(const mc::EntitySnapshot& e, const mc::PlayerSnap& p,
                                double now, float W, float H,
                                ImVec2 out[8], bool vis[8]) {
    const proj::Vec3 cam{ p.x, p.y + 1.62, p.z };
    const proj::Vec3 pts[8] = {
        { e.min_x, e.min_y, e.min_z }, { e.max_x, e.min_y, e.min_z },
        { e.max_x, e.min_y, e.max_z }, { e.min_x, e.min_y, e.max_z },
        { e.min_x, e.max_y, e.min_z }, { e.max_x, e.max_y, e.min_z },
        { e.max_x, e.max_y, e.max_z }, { e.min_x, e.max_y, e.max_z },
    };
    float fov = 70.f;
    mc::read_fov(fov);
    for (int i = 0; i < 8; ++i) {
        vis[i] = proj::world_to_screen(pts[i], cam, p.yaw, p.pitch, fov, W, H,
                                       out[i].x, out[i].y);
        if (!vis[i]) return false;
    }
    return true;
}

void Esp::update(const mc::PlayerSnap&, const mc::WorldSnap&, double) {}

void Esp::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;

    for (const auto& e : w.players) {
        ImVec2 pts[8]; bool vis[8];
        if (!project_box_corners(e, p, now, W.x, W.y, pts, vis)) continue;

        float min_x = pts[0].x, min_y = pts[0].y, max_x = pts[0].x, max_y = pts[0].y;
        for (int i = 1; i < 8; ++i) {
            min_x = std::fmin(min_x, pts[i].x); max_x = std::fmax(max_x, pts[i].x);
            min_y = std::fmin(min_y, pts[i].y); max_y = std::fmax(max_y, pts[i].y);
        }
        const float hpf = std::fmin(std::fmax(e.health / 20.f, 0.f), 1.f);
        const ImU32 col = theme::mix(theme::ACCENT_DEEP, theme::ACCENT, hpf);

        // layered glow box
        uikit::glow_rect(dl, ImVec2(min_x, min_y), ImVec2(max_x, max_y), 2.f,
                         col, 0.5f * opacity, 0.10f * opacity);
        dl->AddRect(ImVec2(min_x, min_y), ImVec2(max_x, max_y),
                    theme::with_alpha(col, 0.9f * opacity), 2.f, 0, 1.4f);

        // health bar (left)
        const float bar_h = max_y - min_y;
        dl->AddRectFilled(ImVec2(min_x - 6.f, min_y), ImVec2(min_x - 3.f, max_y),
                          IM_COL32(10, 8, 14, 200), 1.5f);
        dl->AddRectFilled(ImVec2(min_x - 6.f, max_y - bar_h * hpf),
                          ImVec2(min_x - 3.f, max_y),
                          theme::mix(theme::BAD, theme::OK, hpf), 1.5f);
    }
}

void Esp::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##esp_op", &opacity, 0.2f, 1.f, "opacity %.2f");
    ImGui::PopItemWidth();
    y += 26.f;
}

} // namespace modules

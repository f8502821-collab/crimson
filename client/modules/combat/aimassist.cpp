#include "client/modules/combat/aimassist.h"
#include "client/jvm/mc.h"
#include <imgui.h>
#include <cmath>

namespace modules {

void AimAssist::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    if (mapping_missing()) { set_enabled(false, now); return; }

    // nearest visible player within range
    const mc::EntitySnapshot* best = nullptr;
    double best_d = range;
    for (const auto& e : w.players) {
        if (!e.alive || e.dist > best_d) continue;
        best_d = e.dist;
        best = &e;
    }
    if (!best) return;

    const double dx = best->x - p.x;
    const double dy = (best->y + 1.35) - (p.y + 1.62);   // aim at chest/head
    const double dz = best->z - p.z;

    const double flat = std::sqrt(dx * dx + dz * dz);
    float target_yaw = (float)(std::atan2(-dx, dz) * 57.2957795);
    float target_pitch = (float)(-std::atan2(dy, flat) * 57.2957795);

    // normalize yaw delta to [-180, 180]
    float dyaw = target_yaw - p.yaw;
    while (dyaw > 180.f) dyaw -= 360.f;
    while (dyaw < -180.f) dyaw += 360.f;

    const float ny = p.yaw + dyaw * smooth;
    const float np = p.pitch + (target_pitch - p.pitch) * smooth;
    mc::set_view_angles(ny, np);
}

void AimAssist::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##aa_range", &range, 3.f, 6.f, "range %.1f");
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y + 26.f));
    ImGui::SliderFloat("##aa_smooth", &smooth, 0.1f, 1.f, "smooth %.2f");
    ImGui::PopItemWidth();
    y += 56.f;
}

} // namespace modules

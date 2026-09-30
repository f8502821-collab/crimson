#include "client/modules/movement/speed.h"
#include "client/jvm/mc.h"
#include <imgui.h>
#include <cmath>

namespace modules {

void Speed::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    const double hspeed = std::sqrt(p.vel.x * p.vel.x + p.vel.z * p.vel.z);
    if (hspeed < 0.05 || hspeed > 1.5) return;   // idle or already boosted too fast
    jobject pl = mc::player();
    if (pl) {
        mc::set_velocity_xyz(pl, p.vel.x * multiplier, p.vel.y, p.vel.z * multiplier);
    }
}

void Speed::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##speed_mul", &multiplier, 1.1f, 5.f, "%.2fx");
    ImGui::PopItemWidth();
    y += 26.f;
}

} // namespace modules

#include "client/modules/combat/antiknockback.h"
#include "client/jvm/mc.h"
#include <imgui.h>

namespace modules {

void AntiKnockback::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    int hurt = 0;
    if (!mc::read_hurt_time(hurt)) return;
    if (hurt > 0 && hurt != last_hurt) {
        // just got hurt: crush the knockback velocity
        jobject pl = mc::player();
        if (pl) mc::set_velocity_xyz(pl, p.vel.x * damping, p.vel.y * 0.2, p.vel.z * damping);
    }
    last_hurt = hurt;
}

void AntiKnockback::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##kb_damp", &damping, 0.f, 1.f, "keep %.2f");
    ImGui::PopItemWidth();
    y += 26.f;
}

} // namespace modules

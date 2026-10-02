#include "client/modules/combat/autoclicker.h"
#include "client/jvm/mc.h"
#include <imgui.h>
#include <cmath>

namespace modules {

void AutoClicker::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    if (now - last_click < 1.0 / (double)cps) return;
    last_click = now;
    mc::do_attack();   // swing only when a target/attack is viable (vanilla cooldown gates it)
}

void AutoClicker::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##ac_cps", &cps, 1.f, 20.f, "cps %.0f");
    ImGui::PopItemWidth();
    y += 26.f;
}

} // namespace modules

#include "client/modules/render/fullbright.h"
#include "client/jvm/mc.h"
#include <imgui.h>

namespace modules {

void Fullbright::on_enable(double now) {
    mc::read_gamma(prev);   // remember the user's gamma
}

void Fullbright::update(const mc::PlayerSnap&, const mc::WorldSnap&, double now) {
    mc::write_gamma(boost);
}

void Fullbright::on_disable(double now) {
    if (prev >= 0.f) mc::write_gamma(prev);
    prev = -1.f;
}

void Fullbright::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##fb_boost", &boost, 1.f, 16.f, "gamma %.1f");
    ImGui::PopItemWidth();
    y += 26.f;
}

} // namespace modules

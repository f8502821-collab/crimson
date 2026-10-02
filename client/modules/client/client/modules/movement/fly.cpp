#include "client/modules/movement/fly.h"
#include "client/jvm/mc.h"
#include "client/loader.h"
#include <imgui.h>
#include <cmath>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace modules {

void Fly::on_enable(double now) {
    // grant the ability server-recognized in singleplayer (abilities sync)
    bool can = false, flying = false;
    mc::read_abilities(can, flying);
    mc::write_abilities(true, true);
}

void Fly::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    // hold space = up, shift = down; always horizontal velocity boost
    bool up = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    bool down = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    const double vy = up ? 0.42 : down ? -0.42 : 0.0;

    jobject pl = mc::player();
    if (pl) {
        // keep horizontal motion, override vertical
        const double vx = p.vel.x, vz = p.vel.z;
        mc::set_velocity_xyz(pl, vx * speed, vy, vz * speed);
    }
}

void Fly::on_disable(double now) {
    // restore vanilla state: revoke flight
    mc::write_abilities(false, false);
}

void Fly::draw_settings(float& y, float right_x) {
    ImGui::SetCursorScreenPos(ImVec2(right_x - 200.f, y));
    ImGui::PushItemWidth(190.f);
    ImGui::SliderFloat("##fly_speed", &speed, 0.5f, 5.f, "speed %.1fx");
    ImGui::PopItemWidth();
    y += 26.f;
}

} // namespace modules

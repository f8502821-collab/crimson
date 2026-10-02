#pragma once
#include "client/modules/module.h"

namespace modules {
struct AimAssist : Module {
    float range = 4.5f;
    float smooth = 0.45f;
    AimAssist() : Module("AimAssist", Category::Combat) { map_key = "e.getYaw"; has_settings = true; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

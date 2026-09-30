#pragma once
#include "client/modules/module.h"

namespace modules {
struct Speed : Module {
    float multiplier = 1.6f;
    Speed() : Module("Speed", Category::Movement) { map_key = "e.setVelocityXYZ"; has_settings = true; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

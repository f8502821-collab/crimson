#pragma once
#include "client/modules/module.h"

namespace modules {
struct AutoClicker : Module {
    float cps = 8.f;
    double last_click = 0;
    AutoClicker() : Module("AutoClicker", Category::Combat) { map_key = "mc.doAttack"; has_settings = true; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

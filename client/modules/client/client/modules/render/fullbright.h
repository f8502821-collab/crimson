#pragma once
#include "client/modules/module.h"

namespace modules {
struct Fullbright : Module {
    float boost = 15.f;
    float prev = -1.f;
    Fullbright() : Module("Fullbright", Category::Render) { map_key = "opt.getGamma"; has_settings = true; }
    void on_enable(double now) override;
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void on_disable(double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

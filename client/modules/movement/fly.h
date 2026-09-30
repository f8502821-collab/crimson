#pragma once
#include "client/modules/module.h"

namespace modules {
struct Fly : Module {
    float speed = 1.8f;
    Fly() : Module("Fly", Category::Movement) { map_key = "ab.allowFlying"; has_settings = true; }
    void on_enable(double now) override;
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void on_disable(double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

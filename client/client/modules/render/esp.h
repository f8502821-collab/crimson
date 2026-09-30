#pragma once
#include "client/modules/module.h"

namespace modules {
struct Esp : Module {
    float opacity = 1.f;
    Esp() : Module("ESP", Category::Render) { map_key = "e.getX"; has_settings = true; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

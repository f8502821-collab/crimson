#pragma once
#include "client/modules/module.h"

namespace modules {
struct Freecam : Module {
    float speed = 1.0f;
    Freecam() : Module("Freecam", Category::Movement) { map_key = "e.setVelocityXYZ"; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void on_disable(double now) override;
};
}

#pragma once
#include "client/modules/module.h"

namespace modules {
struct Tracers : Module {
    Tracers() : Module("Tracers", Category::Render) { map_key = "e.getX"; }
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) override;
};
}

#pragma once
#include "client/modules/module.h"

namespace modules {
struct Glow : Module {
    Glow() : Module("Glow", Category::Render) { map_key = "e.getBBox"; }
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) override;
};
}

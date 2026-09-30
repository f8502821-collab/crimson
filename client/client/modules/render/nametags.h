#pragma once
#include "client/modules/module.h"

namespace modules {
struct Nametags : Module {
    Nametags() : Module("Nametags", Category::Render) { map_key = "e.getX"; }
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) override;
};
}

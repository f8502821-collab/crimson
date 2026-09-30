#pragma once
#include "client/modules/module.h"

namespace modules {
struct ChestEsp : Module {
    ChestEsp() : Module("ChestESP", Category::Render) { map_key = "w.getBlockEntities"; }
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) override;
};
}

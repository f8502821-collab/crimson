#pragma once
#include "client/modules/module.h"

namespace modules {
struct ItemEsp : Module {
    ItemEsp() : Module("ItemESP", Category::Render) { map_key = "w.getEntities"; }
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) override;
};
}

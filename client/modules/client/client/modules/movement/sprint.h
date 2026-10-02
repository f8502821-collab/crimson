#pragma once
#include "client/modules/module.h"

namespace modules {
struct Sprint : Module {
    Sprint() : Module("Sprint", Category::Movement) { map_key = "e.setSprinting"; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
};
}

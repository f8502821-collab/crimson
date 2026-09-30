#pragma once
#include "client/modules/module.h"

namespace modules {
struct NoFall : Module {
    NoFall() : Module("NoFall", Category::Movement) { map_key = "e.fallDistance"; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
};
}

#pragma once
#include "client/modules/module.h"

namespace modules {
struct TimeOverride : Module {
    long time = 1000;      // dawn by default
    TimeOverride() : Module("Time Override", Category::Misc) { map_key = "w.setTime"; }
    void on_enable(double now) override;
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
};
}

#pragma once
#include "client/modules/module.h"

namespace modules {
struct WeatherOverride : Module {
    WeatherOverride() : Module("Weather Override", Category::Misc) { map_key = "w.setRain"; }
    void on_enable(double now) override;
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
};
}

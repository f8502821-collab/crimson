#include "client/modules/misc/weatheroverride.h"
#include "client/jvm/mc.h"

namespace modules {

void WeatherOverride::on_enable(double now) {
    mc::world_set_rain(false);
}

void WeatherOverride::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    mc::world_set_rain(false);   // keep clear while enabled
}

} // namespace modules

#include "client/modules/misc/timeoverride.h"
#include "client/jvm/mc.h"

namespace modules {

void TimeOverride::on_enable(double now) {
    mc::world_set_time(time);
}

void TimeOverride::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    mc::world_set_time(time);   // keep pinned while enabled
}

} // namespace modules

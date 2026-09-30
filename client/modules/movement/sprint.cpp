#include "client/modules/movement/sprint.h"
#include "client/jvm/mc.h"
#include <cmath>

namespace modules {

void Sprint::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    // sprint whenever moving forward-ish (any horizontal motion at all)
    const double hspeed = std::sqrt(p.vel.x * p.vel.x + p.vel.z * p.vel.z);
    if (hspeed > 0.05) {
        jobject pl = mc::player();
        if (pl) mc::set_sprinting(pl, true);
    }
}

} // namespace modules

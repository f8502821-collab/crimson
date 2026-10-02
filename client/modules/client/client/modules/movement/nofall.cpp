#include "client/modules/movement/nofall.h"
#include "client/jvm/mc.h"

namespace modules {

void NoFall::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    double fd = 0;
    if (mc::read_fall_distance(fd) && fd > 0.0)
        mc::write_fall_distance(0.0);
}

} // namespace modules

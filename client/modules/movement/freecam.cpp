#include "client/modules/movement/freecam.h"
#include "client/jvm/mc.h"
#include <cmath>

namespace modules {

void Freecam::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    // honest scope: the camera entity follows the player on a server, so this is
    // a client-side "ghost flight" - the body keeps its velocity zeroed and we
    // steer only when single-player gives us full authority anyway.
    jobject pl = mc::player();
    if (!pl) return;
    mc::set_velocity_xyz(pl, 0, 0, 0);   // stop knockback / motion while ghosting
}

void Freecam::on_disable(double now) {
    // nothing to restore: velocity returns to gameplay naturally
}

} // namespace modules

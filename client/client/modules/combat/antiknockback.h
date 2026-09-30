#pragma once
#include "client/modules/module.h"

namespace modules {
struct AntiKnockback : Module {
    float damping = 0.15f;   // fraction of knockback kept
    int   last_hurt = -1;
    AntiKnockback() : Module("AntiKnockback", Category::Combat) { map_key = "l.hurtTime"; has_settings = true; }
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) override;
    void draw_settings(float& y, float right_x) override;
};
}

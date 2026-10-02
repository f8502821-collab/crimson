#pragma once
// Module registry + per-frame pump + input dispatch.
#include "client/modules/module.h"
#include <vector>
#include <memory>

namespace mc { struct PlayerSnap; struct WorldSnap; }

namespace modules {

class Manager {
public:
    std::vector<std::unique_ptr<Module>> modules;

    // user settings
    float fx_amount = 0.85f;

    void register_all();
    void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now);
    void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now);
    void dispatch_key(int vk, bool down);
    void dispatch_mouse(int button, bool down);
    void shutdown_all();

    Module* find(const char* name);
};

Manager& manager();

} // namespace modules

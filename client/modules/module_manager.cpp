#include "client/modules/module_manager.h"
#include "client/modules/module.h"
#include "client/loader.h"
#include "client/modules/render/esp.h"
#include "client/modules/render/tracers.h"
#include "client/modules/render/nametags.h"
#include "client/modules/render/chestesp.h"
#include "client/modules/render/itemesp.h"
#include "client/modules/render/glow.h"
#include "client/modules/render/fullbright.h"
#include "client/modules/movement/fly.h"
#include "client/modules/movement/sprint.h"
#include "client/modules/movement/speed.h"
#include "client/modules/movement/nofall.h"
#include "client/modules/movement/freecam.h"
#include "client/modules/combat/aimassist.h"
#include "client/modules/combat/autoclicker.h"
#include "client/modules/combat/antiknockback.h"
#include "client/modules/misc/timeoverride.h"
#include "client/modules/misc/weatheroverride.h"
#include "common/log.h"
#include <cstring>

namespace modules {

static const char* TAG_INIT = "modules";

static Manager g_mgr;

Manager& manager() { return g_mgr; }

void Manager::register_all() {
    modules.push_back(std::make_unique<Esp>());
    modules.push_back(std::make_unique<Tracers>());
    modules.push_back(std::make_unique<Nametags>());
    modules.push_back(std::make_unique<ChestEsp>());
    modules.push_back(std::make_unique<ItemEsp>());
    modules.push_back(std::make_unique<Glow>());
    modules.push_back(std::make_unique<Fullbright>());
    modules.push_back(std::make_unique<Fly>());
    modules.push_back(std::make_unique<Sprint>());
    modules.push_back(std::make_unique<Speed>());
    modules.push_back(std::make_unique<NoFall>());
    modules.push_back(std::make_unique<Freecam>());
    modules.push_back(std::make_unique<AimAssist>());
    modules.push_back(std::make_unique<AutoClicker>());
    modules.push_back(std::make_unique<AntiKnockback>());
    modules.push_back(std::make_unique<TimeOverride>());
    modules.push_back(std::make_unique<WeatherOverride>());

    // default binds
    find("ESP")->keybind = 'B';
    find("Tracers")->keybind = 'N';
    find("Fullbright")->keybind = 'V';
    clog::info(TAG_INIT, "%d modules registered", (int)modules.size());
}

void Manager::update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {
    for (auto& m : modules)
        if (m->enabled) m->update(p, w, now);
}

void Manager::render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {
    for (auto& m : modules)
        if (m->enabled) m->render(w, p, now);
}

void Manager::dispatch_key(int vk, bool down) {
    for (auto& m : modules) {
        if (m->on_key(vk, down)) continue;
    }
}

void Manager::dispatch_mouse(int button, bool down) {
    for (auto& m : modules)
        if (m->enabled) m->on_mouse(button, down);
}

void Manager::shutdown_all() {
    for (auto& m : modules)
        if (m->enabled) m->set_enabled(false, 0);
    modules.clear();
}

Module* Manager::find(const char* name) {
    for (auto& m : modules)
        if (strcmp(m->name, name) == 0) return m.get();
    return nullptr;
}

} // namespace modules

#include "client/modules/module.h"
#include "client/modules/module_manager.h"
#include "client/jvm/mappings.h"
#include "client/gui/widgets.h"
#include "client/loader.h"
#include <cstring>
#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace modules {

const char* category_name(Category c) {
    switch (c) {
    case Category::Combat:  return "Combat";
    case Category::Movement:return "Movement";
    case Category::Render:  return "Render";
    case Category::Misc:    return "Misc";
    default:                return "?";
    }
}

Module::Module(const char* n, Category c)
    : name(n), category(category_name(c)), cat(c), map_key("") {}

void Module::set_enabled(bool v, double now) {
    if (enabled == v) return;
    enabled = v;
    if (v) on_enable(now);
    else   on_disable(now);
    char msg[128];
    snprintf(msg, sizeof(msg), "%s %s", name, v ? "ON" : "OFF");
    widgets::toast(msg);
}

bool Module::on_key(int vk, bool down) {
    if (binding && down) {
        keybind = (vk == VK_ESCAPE) ? 0 : vk;   // ESC clears the bind
        binding = false;
        return true;
    }
    if (keybind && vk == keybind && down) {
        set_enabled(!enabled, client::now_sec());
        return true;
    }
    return false;
}

bool Module::mapping_missing() {
    if (map_key.empty()) return false;
    return !mappings::ok(map_key.c_str());
}

} // namespace modules

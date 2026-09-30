#include "client/util/config.h"
#include "client/modules/module_manager.h"
#include "client/modules/module.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>
#include <cstdio>

namespace config {

static const char* TAG = "config";

static std::string path() {
    char buf[MAX_PATH]{};
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, buf)))
        return std::string(buf) + "\\Crimson\\config.ini";
    return "crimson_config.ini";
}

void save() {
    const std::string p = path();
    CreateDirectoryA((p.substr(0, p.find_last_of("\\/"))).c_str(), nullptr);
    FILE* f = nullptr;
    if (fopen_s(&f, p.c_str(), "w") != 0 || !f) {
        clog::warn(TAG, "cannot write config: %s", p.c_str());
        return;
    }
    auto& mgr = modules::manager();
    fprintf(f, "fx_amount=%.3f\n", mgr.fx_amount);
    for (auto& m : mgr.modules) {
        fprintf(f, "mod.%s.enabled=%d\n", m->name, m->enabled ? 1 : 0);
        fprintf(f, "mod.%s.key=%d\n", m->name, m->keybind);
    }
    fclose(f);
    clog::info(TAG, "saved %s", p.c_str());
}

void load() {
    const std::string p = path();
    FILE* f = nullptr;
    if (fopen_s(&f, p.c_str(), "r") != 0 || !f) return;   // first run
    char line[256];
    auto& mgr = modules::manager();
    while (fgets(line, sizeof(line), f)) {
        char key[128]{}, val[64]{};
        if (sscanf_s(line, "%127[^=]=%63s", key, (unsigned)sizeof(key), val, (unsigned)sizeof(val)) != 2)
            continue;
        if (strcmp(key, "fx_amount") == 0) {
            mgr.fx_amount = (float)atof(val);
        } else if (strncmp(key, "mod.", 4) == 0) {
            char name[96]{};
            char field[32]{};
            if (sscanf_s(key + 4, "%95[^.].%31s", name, (unsigned)sizeof(name), field, (unsigned)sizeof(field)) == 2) {
                if (Module* m = mgr.find(name)) {
                    if (strcmp(field, "enabled") == 0) m->enabled = atoi(val) != 0;
                    else if (strcmp(field, "key") == 0) m->keybind = atoi(val);
                }
            }
        }
    }
    fclose(f);
    clog::info(TAG, "loaded %s", p.c_str());
}

} // namespace config

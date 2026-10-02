#pragma once
// Module base: every feature derives from this. Modules never touch JNIEnv;
// they consume snapshots and call the typed access layer in jvm/mc.h.
#include <cstdint>
#include <string>

namespace mc { struct PlayerSnap; struct WorldSnap; }

namespace modules {

enum class Category : int { Combat = 0, Movement, Render, Misc, COUNT };

const char* category_name(Category c);

struct Module {
    const char* name;
    const char* category;      // display category string
    Category    cat;
    bool        enabled = false;
    int         keybind = 0;   // 0 = unbound
    bool        binding = false;   // listening for a key capture
    bool        has_settings = false;
    std::string map_key;       // mappings key this module depends on ("" = none)

    Module(const char* n, Category c);
    virtual ~Module() = default;

    virtual void on_enable(double now) {}
    virtual void on_disable(double now) {}
    virtual void update(const mc::PlayerSnap& p, const mc::WorldSnap& w, double now) {}
    virtual void render(const mc::WorldSnap& w, const mc::PlayerSnap& p, double now) {}
    virtual void draw_settings(float& y, float right_x) {}

    // internal: flip state + callbacks + toast
    void set_enabled(bool v, double now);

    // key/mouse dispatch (default: keybind toggle)
    virtual bool on_key(int vk, bool down);
    virtual void on_mouse(int button, bool down) {}

protected:
    // helper for subclasses: true if mapping missing -> auto-disable once
    bool mapping_missing();
};

} // namespace modules

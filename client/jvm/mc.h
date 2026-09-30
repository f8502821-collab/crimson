#pragma once
// Typed game-state access. The ONLY layer allowed to touch JNIEnv.
// Every getter returns a plain snapshot; failures yield empty optionals.
#include "client/jvm/jni_min.h"
#include <vector>
#include <string>

namespace mc {

struct Vec3 { double x = 0, y = 0, z = 0; };

struct EntitySnapshot {
    jobject  ref = nullptr;   // do NOT cache between frames
    double   x = 0, y = 0, z = 0;
    double   min_x = 0, min_y = 0, min_z = 0;
    double   max_x = 0, max_y = 0, max_z = 0;
    float    health = 0.f;
    bool     alive = false;
    bool     is_player = false;
    bool     is_item = false;
    bool     is_chest = false;
    std::string name;
    double   dist = 0.0;      // filled by snap_players
};

// Per-frame world data pulled on the render thread.
struct WorldSnap {
    bool                          valid = false;
    std::vector<EntitySnapshot>   players;
    std::vector<EntitySnapshot>   items;
    struct Chest {
        int x = 0, y = 0, z = 0;
        double dist = 0.0;
    };
    std::vector<Chest>            chests;
    float                         gamma = -1.f;  // <0 = unavailable
    float                         fov = -1.f;
    long                          world_time = -1;
    bool                          raining = false;
};

// Player context passed to modules each frame.
struct PlayerSnap {
    bool   valid = false;
    double x = 0, y = 0, z = 0;
    float  yaw = 0.f, pitch = 0.f;
    float  health = 0.f;
    bool   on_ground = false;
    Vec3   vel;
};

// ---- accessors (each internally exception-guarded) ------------------------
jobject    minecraft();                  // MinecraftClient.instance
jobject    player();                     // ClientPlayerEntity
jobject    world();                      // ClientWorld

bool       read_player(PlayerSnap& out);         // false if unavailable
bool       read_world(WorldSnap& out);           // aggregates players/items/chests
bool       read_gamma(float& out);               // options gamma
bool       write_gamma(float v);
bool       read_fov(float& out);
bool       read_velocity(jobject entity, Vec3& out);
bool       set_velocity_xyz(jobject entity, double x, double y, double z);
bool       set_position(jobject entity, double x, double y, double z); // best effort
bool       set_sprinting(jobject player_ref, bool on);
bool       read_fall_distance(double& out);
bool       write_fall_distance(double v);
bool       read_abilities(bool& can_fly, bool& flying);
bool       write_abilities(bool can_fly, bool flying);
bool       set_view_angles(float yaw, float pitch);
bool       do_attack();                  // mc.doAttack()
bool       read_hurt_time(int& out);     // local player hurtTime
bool       world_set_time(long t);
bool       world_set_rain(bool on);

} // namespace mc

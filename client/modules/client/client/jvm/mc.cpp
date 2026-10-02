#include "client/jvm/mc.h"
#include "client/jvm/jvm.h"
#include "client/jvm/mappings.h"
#include "common/log.h"
#include <cmath>
#include <cstring>

namespace mc {

static const char* TAG = "mc";

// ---------------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------------

// java/lang/Double.valueOf(D)Ljava/lang/Double;  (platform class, always there)
static jobject boxed_double(JNIEnv* e, double v) {
    jclass d = e->FindClass(e, "java/lang/Double");
    if (jvm::check_exc(e) || !d) return nullptr;
    jmethodID mid = e->GetStaticMethodID(e, d, "valueOf", "(D)Ljava/lang/Double;");
    if (jvm::check_exc(e) || !mid) return nullptr;
    return e->CallStaticObjectMethod(e, d, mid, v);
}

static double unbox_double(JNIEnv* e, jobject boxed) {
    if (!boxed) return 0.0;
    jmethodID mid = e->GetMethodID(e, e->GetObjectClass(e, boxed), "doubleValue", "()D");
    if (jvm::check_exc(e) || !mid) return 0.0;
    return e->CallDoubleMethod(e, boxed, mid);
}

// Read a GameOptions value that may be a legacy double field or a SimpleOption.
static bool option_read(jfieldID legacy, jmethodID getter, float& out) {
    JNIEnv* e = jvm::env();
    jobject m = minecraft();
    if (!e || !m || !mappings::M.mc_options) return false;
    jobject opts = e->GetObjectField(e, m, mappings::M.mc_options);
    if (jvm::check_exc(e) || !opts) return false;
    jvm::Frame f(e);
    if (!f.ok) return false;

    if (getter) {
        jobject so = e->CallObjectMethod(e, opts, getter);
        if (!jvm::check_exc(e) && so) {
            // SimpleOption value: try intermediary then plain "get"
            static jmethodID get_m = nullptr;
            if (!get_m) {
                jclass c = e->GetObjectClass(e, so);
                get_m = e->GetMethodID(e, c, "method_44213", "()Ljava/lang/Object;");
                if (!get_m) { jvm::check_exc(e); get_m = e->GetMethodID(e, c, "get", "()Ljava/lang/Object;"); }
                jvm::check_exc(e);
            }
            if (get_m) {
                jobject v = e->CallObjectMethod(e, so, get_m);
                if (!jvm::check_exc(e) && v) { out = (float)unbox_double(e, v); return true; }
            }
        }
    }
    if (legacy) {
        out = (float)e->GetDoubleField(e, opts, legacy);
        return !jvm::check_exc(e);
    }
    return false;
}

static jobject abilities_of(JNIEnv* e, jobject p) {
    if (!e || !p) return nullptr;
    if (mappings::M.p_getAbilities) {
        jobject a = e->CallObjectMethod(e, p, mappings::M.p_getAbilities);
        if (!jvm::check_exc(e) && a) return a;
    }
    if (mappings::M.p_abilities) {
        jobject a = e->GetObjectField(e, p, mappings::M.p_abilities);
        if (!jvm::check_exc(e)) return a;
    }
    return nullptr;
}

// Cached java.lang iterator plumbing
struct IterIds { jclass cls = nullptr; jmethodID it{}, has{}, next{}; };
static IterIds g_iter;

static bool iter_ids(JNIEnv* e) {
    if (g_iter.it) return true;
    g_iter.cls = e->FindClass(e, "java/lang/Iterable");
    if (!g_iter.cls || jvm::check_exc(e)) return false;
    g_iter.it   = e->GetMethodID(e, g_iter.cls, "iterator", "()Ljava/util/Iterator;");
    g_iter.has  = e->GetMethodID(e, g_iter.cls, "hasNext", "()Z");
    g_iter.next = e->GetMethodID(e, g_iter.cls, "next", "()Ljava/lang/Object;");
    return g_iter.it && g_iter.has && g_iter.next && !jvm::check_exc(e);
}

// Best-effort entity display name; empty on mapping miss.
static std::string entity_name(JNIEnv* e, jobject obj) {
    jvm::Frame f(e);
    if (!f.ok) return {};
    jmethodID getName = e->GetMethodID(e, mappings::M.entity, "method_5477",
                                       "()Lnet/minecraft/class_2585;");
    if (!getName) { jvm::check_exc(e); getName = e->GetMethodID(e, mappings::M.entity, "getName", "()Lnet/minecraft/class_2585;"); }
    jvm::check_exc(e);
    if (!getName) return {};
    jobject text = e->CallObjectMethod(e, obj, getName);
    if (jvm::check_exc(e) || !text) return {};
    jmethodID gs = e->GetMethodID(e, e->GetObjectClass(e, text), "method_44717",
                                  "()Ljava/lang/String;");
    if (!gs) { jvm::check_exc(e); gs = e->GetMethodID(e, e->GetObjectClass(e, text), "getString", "()Ljava/lang/String;"); }
    jvm::check_exc(e);
    if (!gs) return {};
    jstring s = (jstring)e->CallObjectMethod(e, text, gs);
    if (jvm::check_exc(e) || !s) return {};
    return jvm::to_utf8(e, s);
}

static bool fill_entity_common(JNIEnv* e, jobject obj, EntitySnapshot& s) {
    if (!e || !obj || !mappings::M.e_getX) return false;
    s.ref = obj;
    s.x = e->CallDoubleMethod(e, obj, mappings::M.e_getX);
    s.y = e->CallDoubleMethod(e, obj, mappings::M.e_getY);
    s.z = e->CallDoubleMethod(e, obj, mappings::M.e_getZ);
    jvm::check_exc(e, "entity pos");

    if (mappings::M.e_getBBox && mappings::M.box &&
        mappings::M.box_minX && mappings::M.box_maxZ) {
        jobject bb = e->CallObjectMethod(e, obj, mappings::M.e_getBBox);
        if (!jvm::check_exc(e) && bb) {
            s.min_x = e->GetDoubleField(e, bb, mappings::M.box_minX);
            s.min_y = e->GetDoubleField(e, bb, mappings::M.box_minY);
            s.min_z = e->GetDoubleField(e, bb, mappings::M.box_minZ);
            s.max_x = e->GetDoubleField(e, bb, mappings::M.box_maxX);
            s.max_y = e->GetDoubleField(e, bb, mappings::M.box_maxY);
            s.max_z = e->GetDoubleField(e, bb, mappings::M.box_maxZ);
        }
    }

    if (mappings::M.e_isAlive)
        s.alive = e->CallBooleanMethod(e, obj, mappings::M.e_isAlive) != 0;
    jvm::check_exc(e, "isAlive");

    if (mappings::M.l_getHealth && mappings::M.living &&
        e->IsInstanceOf(e, obj, mappings::M.living))
        s.health = e->CallFloatMethod(e, obj, mappings::M.l_getHealth);
    jvm::check_exc(e, "health");

    s.is_player = mappings::M.otherPlayer && mappings::M.living &&
                  e->IsInstanceOf(e, obj, mappings::M.otherPlayer);
    s.is_item   = mappings::M.itemEntity && mappings::M.living &&
                  e->IsInstanceOf(e, obj, mappings::M.itemEntity);
    if (s.is_player) s.name = entity_name(e, obj);
    return true;
}

// ---------------------------------------------------------------------------
// base accessors
// ---------------------------------------------------------------------------

jobject minecraft() {
    JNIEnv* e = jvm::env();
    if (!e || !mappings::M.mc_getInstance || !mappings::M.mc) return nullptr;
    jobject m = e->CallStaticObjectMethod(e, mappings::M.mc, mappings::M.mc_getInstance);
    jvm::check_exc(e, "mc.getInstance");
    return m;
}

jobject player() {
    JNIEnv* e = jvm::env();
    jobject m = minecraft();
    if (!e || !m || !mappings::M.mc_player) return nullptr;
    jobject p = e->GetObjectField(e, m, mappings::M.mc_player);
    jvm::check_exc(e, "mc.player");
    return p;
}

jobject world() {
    JNIEnv* e = jvm::env();
    jobject m = minecraft();
    if (!e || !m || !mappings::M.mc_world) return nullptr;
    jobject w = e->GetObjectField(e, m, mappings::M.mc_world);
    jvm::check_exc(e, "mc.world");
    return w;
}

bool read_player(PlayerSnap& out) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p || !mappings::M.e_getX) return false;
    jvm::Frame f(e);
    if (!f.ok) return false;

    out = {};
    out.valid = true;
    out.x = e->CallDoubleMethod(e, p, mappings::M.e_getX);
    out.y = e->CallDoubleMethod(e, p, mappings::M.e_getY);
    out.z = e->CallDoubleMethod(e, p, mappings::M.e_getZ);
    jvm::check_exc(e, "player pos");

    if (mappings::M.e_getYaw)  out.yaw  = e->CallFloatMethod(e, p, mappings::M.e_getYaw);
    else if (mappings::M.e_yawF) out.yaw = e->GetFloatField(e, p, mappings::M.e_yawF);
    if (mappings::M.e_getPitch) out.pitch = e->CallFloatMethod(e, p, mappings::M.e_getPitch);
    else if (mappings::M.e_pitchF) out.pitch = e->GetFloatField(e, p, mappings::M.e_pitchF);
    jvm::check_exc(e, "player angles");

    if (mappings::M.l_getHealth && e->IsInstanceOf(e, p, mappings::M.living))
        out.health = e->CallFloatMethod(e, p, mappings::M.l_getHealth);
    jvm::check_exc(e, "player health");

    if (mappings::M.e_getVelocity) {
        jobject v = e->CallObjectMethod(e, p, mappings::M.e_getVelocity);
        if (!jvm::check_exc(e) && v && mappings::M.v_x) {
            out.vel.x = e->GetDoubleField(e, v, mappings::M.v_x);
            out.vel.y = e->GetDoubleField(e, v, mappings::M.v_y);
            out.vel.z = e->GetDoubleField(e, v, mappings::M.v_z);
        }
    }
    return true;
}

bool read_world(WorldSnap& out) {
    JNIEnv* e = jvm::env();
    jobject w = world();
    jobject p = player();
    if (!e || !w || !iter_ids(e)) return false;
    jvm::Frame f(e);
    if (!f.ok) return false;

    out = {};
    out.valid = true;

    double px = 0, py = 0, pz = 0;
    if (p && mappings::M.e_getX) {
        px = e->CallDoubleMethod(e, p, mappings::M.e_getX);
        py = e->CallDoubleMethod(e, p, mappings::M.e_getY);
        pz = e->CallDoubleMethod(e, p, mappings::M.e_getZ);
        jvm::check_exc(e, "self pos");
    }

    // ---- entities ----
    if (mappings::M.w_getEntities) {
        jobject iterable = e->CallObjectMethod(e, w, mappings::M.w_getEntities);
        if (!jvm::check_exc(e) && iterable) {
            jobject it = e->CallObjectMethod(e, iterable, g_iter.it);
            jvm::check_exc(e);
            int guard = 0;
            while (it && guard++ < 4096) {
                if (!e->CallBooleanMethod(e, it, g_iter.has)) break;
                jvm::check_exc(e);
                jobject obj = e->CallObjectMethod(e, it, g_iter.next);
                if (jvm::check_exc(e) || !obj) break;

                // skip the local player itself
                if (p && e->IsSameObject(e, obj, p)) { e->DeleteLocalRef(e, obj); continue; }

                EntitySnapshot s;
                if (fill_entity_common(e, obj, s)) {
                    const double dx = s.x - px, dy = s.y - py, dz = s.z - pz;
                    s.dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                    if (s.is_player && s.alive) out.players.push_back(s);
                    else if (s.is_item && s.dist < 128.0) out.items.push_back(s);
                }
                e->DeleteLocalRef(e, obj);
            }
        }
    }

    // ---- chest block entities ----
    if (mappings::M.w_getBlockEntities && mappings::M.chestBE &&
        mappings::M.be_getPos && mappings::M.bp_getX) {
        jobject coll = e->CallObjectMethod(e, w, mappings::M.w_getBlockEntities);
        if (!jvm::check_exc(e) && coll) {
            jobject it = e->CallObjectMethod(e, coll, g_iter.it);
            jvm::check_exc(e);
            int guard = 0;
            while (it && guard++ < 4096) {
                if (!e->CallBooleanMethod(e, it, g_iter.has)) break;
                jvm::check_exc(e);
                jobject obj = e->CallObjectMethod(e, it, g_iter.next);
                if (jvm::check_exc(e) || !obj) break;

                if (e->IsInstanceOf(e, obj, mappings::M.chestBE)) {
                    jobject pos = e->CallObjectMethod(e, obj, mappings::M.be_getPos);
                    if (!jvm::check_exc(e) && pos) {
                        WorldSnap::Chest c;
                        c.x = e->CallIntMethod(e, pos, mappings::M.bp_getX);
                        c.y = e->CallIntMethod(e, pos, mappings::M.bp_getY);
                        c.z = e->CallIntMethod(e, pos, mappings::M.bp_getZ);
                        const double dx = (c.x + 0.5) - px;
                        const double dy = (c.y + 0.5) - py;
                        const double dz = (c.z + 0.5) - pz;
                        c.dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                        if (c.dist < 128.0) out.chests.push_back(c);
                    }
                }
                e->DeleteLocalRef(e, obj);
            }
        }
    }

    // ---- options mirrored into the snapshot ----
    option_read(mappings::M.opt_gamma, mappings::M.opt_getGamma, out.gamma);
    option_read(mappings::M.opt_fov, mappings::M.opt_getFov, out.fov);
    return true;
}

bool read_gamma(float& out)  { return option_read(mappings::M.opt_gamma, mappings::M.opt_getGamma, out); }

bool write_gamma(float v) {
    JNIEnv* e = jvm::env();
    jobject m = minecraft();
    if (!e || !m || !mappings::M.mc_options) return false;
    jobject opts = e->GetObjectField(e, m, mappings::M.mc_options);
    if (jvm::check_exc(e) || !opts) return false;
    jvm::Frame f(e);
    if (!f.ok) return false;

    if (mappings::M.opt_getGamma) {
        jobject so = e->CallObjectMethod(e, opts, mappings::M.opt_getGamma);
        if (!jvm::check_exc(e) && so) {
            static jmethodID set_m = nullptr;
            if (!set_m) {
                jclass c = e->GetObjectClass(e, so);
                set_m = e->GetMethodID(e, c, "method_45543", "(Ljava/lang/Object;)V");
                if (!set_m) { jvm::check_exc(e); set_m = e->GetMethodID(e, c, "set", "(Ljava/lang/Object;)V"); }
                jvm::check_exc(e);
            }
            if (set_m) {
                jobject boxed = boxed_double(e, v);
                if (boxed) {
                    e->CallVoidMethod(e, so, set_m, boxed);
                    return !jvm::check_exc(e, "setGamma");
                }
            }
        }
    }
    if (mappings::M.opt_gamma) {
        e->SetDoubleField(e, opts, mappings::M.opt_gamma, (double)v);
        return !jvm::check_exc(e, "setGamma legacy");
    }
    return false;
}

bool read_fov(float& out) { return option_read(mappings::M.opt_fov, mappings::M.opt_getFov, out); }

bool read_velocity(jobject entity, Vec3& out) {
    JNIEnv* e = jvm::env();
    if (!e || !entity || !mappings::M.e_getVelocity) return false;
    jobject v = e->CallObjectMethod(e, entity, mappings::M.e_getVelocity);
    if (jvm::check_exc(e) || !v || !mappings::M.v_x) return false;
    out.x = e->GetDoubleField(e, v, mappings::M.v_x);
    out.y = e->GetDoubleField(e, v, mappings::M.v_y);
    out.z = e->GetDoubleField(e, v, mappings::M.v_z);
    jvm::check_exc(e, "velocity");
    return true;
}

bool set_velocity_xyz(jobject entity, double x, double y, double z) {
    JNIEnv* e = jvm::env();
    if (!e || !entity || !mappings::M.e_setVelocityXYZ) return false;
    e->CallVoidMethod(e, entity, mappings::M.e_setVelocityXYZ, x, y, z);
    return !jvm::check_exc(e, "setVelocity");
}

bool set_position(jobject entity, double x, double y, double z) {
    // low-confidence dynamic attempt: Entity.setPosition(DDD)V
    JNIEnv* e = jvm::env();
    if (!e || !entity) return false;
    jmethodID mid = e->GetMethodID(e, mappings::M.entity, "method_24201", "(DDD)V");
    if (!mid) { jvm::check_exc(e); mid = e->GetMethodID(e, mappings::M.entity, "setPosition", "(DDD)V"); }
    jvm::check_exc(e);
    if (!mid) return false;
    e->CallVoidMethod(e, entity, mid, x, y, z);
    return !jvm::check_exc(e, "setPosition");
}

bool set_sprinting(jobject player_ref, bool on) {
    JNIEnv* e = jvm::env();
    if (!e || !player_ref || !mappings::M.e_setSprinting) return false;
    e->CallVoidMethod(e, player_ref, mappings::M.e_setSprinting, (jboolean)(on ? 1 : 0));
    return !jvm::check_exc(e, "setSprinting");
}

bool read_fall_distance(double& out) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p || !mappings::M.e_fallDistance) return false;
    out = e->GetDoubleField(e, p, mappings::M.e_fallDistance);
    if (jvm::check_exc(e)) {
        // float legacy field shares the resolved slot only if D failed;
        // a D miss resolves to nullptr so this branch is only for D ok
        return false;
    }
    return true;
}

bool write_fall_distance(double v) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p || !mappings::M.e_fallDistance) return false;
    e->SetDoubleField(e, p, mappings::M.e_fallDistance, v);
    return !jvm::check_exc(e, "fallDistance");
}

bool read_abilities(bool& can_fly, bool& flying) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p) return false;
    jvm::Frame f(e);
    if (!f.ok) return false;
    jobject a = abilities_of(e, p);
    if (!a || !mappings::M.ab_allowFlying || !mappings::M.ab_flying) return false;
    can_fly = e->GetBooleanField(e, a, mappings::M.ab_allowFlying) != 0;
    flying  = e->GetBooleanField(e, a, mappings::M.ab_flying) != 0;
    return !jvm::check_exc(e, "abilities");
}

bool write_abilities(bool can_fly, bool flying) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p) return false;
    jvm::Frame f(e);
    if (!f.ok) return false;
    jobject a = abilities_of(e, p);
    if (!a || !mappings::M.ab_allowFlying || !mappings::M.ab_flying) return false;
    e->SetBooleanField(e, a, mappings::M.ab_allowFlying, (jboolean)(can_fly ? 1 : 0));
    e->SetBooleanField(e, a, mappings::M.ab_flying, (jboolean)(flying ? 1 : 0));
    return !jvm::check_exc(e, "write abilities");
}

bool set_view_angles(float yaw, float pitch) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p) return false;
    bool any = false;
    if (mappings::M.e_setYaw) {
        e->CallVoidMethod(e, p, mappings::M.e_setYaw, yaw);
        any = !jvm::check_exc(e, "setYaw");
    } else if (mappings::M.e_yawF) {
        e->SetFloatField(e, p, mappings::M.e_yawF, yaw);
        any = !jvm::check_exc(e, "yawF");
    }
    if (mappings::M.e_setPitch) {
        e->CallVoidMethod(e, p, mappings::M.e_setPitch, pitch);
        any = any && !jvm::check_exc(e, "setPitch");
    } else if (mappings::M.e_pitchF) {
        e->SetFloatField(e, p, mappings::M.e_pitchF, pitch);
        any = any && !jvm::check_exc(e, "pitchF");
    }
    return any;
}

bool do_attack() {
    JNIEnv* e = jvm::env();
    jobject m = minecraft();
    if (!e || !m || !mappings::M.mc_doAttack) return false;
    e->CallBooleanMethod(e, m, mappings::M.mc_doAttack);
    return !jvm::check_exc(e, "doAttack");
}

bool read_hurt_time(int& out) {
    JNIEnv* e = jvm::env();
    jobject p = player();
    if (!e || !p || !mappings::M.l_hurtTime) return false;
    out = e->GetIntField(e, p, mappings::M.l_hurtTime);
    return !jvm::check_exc(e, "hurtTime");
}

bool world_set_time(long t) {
    JNIEnv* e = jvm::env();
    jobject w = world();
    if (!e || !w || !mappings::M.w_setTime) return false;
    e->CallVoidMethod(e, w, mappings::M.w_setTime, (jlong)t);
    return !jvm::check_exc(e, "setTime");
}

bool world_set_rain(bool on) {
    JNIEnv* e = jvm::env();
    jobject w = world();
    if (!e || !w || !mappings::M.w_setRain) return false;
    e->CallVoidMethod(e, w, mappings::M.w_setRain, (jfloat)(on ? 1.0f : 0.0f));
    return !jvm::check_exc(e, "setRain");
}

} // namespace mc

#include "client/jvm/mappings.h"
#include "client/jvm/jvm.h"
#include "common/log.h"
#include <cstring>
#include <initializer_list>

namespace mappings {

All M;

static const char* TAG = "maps";
static std::vector<std::string> g_report;

bool ok(const char* key) {
    // report lines look like "key OK detail" / "key MISS (no owner class)"
    for (const auto& r : g_report) {
        if (r.compare(0, strlen(key), key) == 0) {
            const char* after = r.c_str() + strlen(key);
            while (*after == ' ') ++after;
            return after[0] == 'O' && after[1] == 'K';
        }
    }
    return false;
}

static void note(const char* key, bool good, const char* detail) {
    char line[224];
    _snprintf_s(line, sizeof(line), _TRUNCATE, "%s %s %s",
                key, good ? "OK" : "MISS", detail[0] ? detail : "-");
    g_report.push_back(line);
    if (good) clog::info(TAG, "%s", line);
    else      clog::warn(TAG, "%s", line);
}

static jclass find_first(JNIEnv* e, const char* key, std::initializer_list<const char*> names) {
    for (const char* n : names) {
        jclass c = e->FindClass(e, n);
        if (jvm::check_exc(e)) continue;
        if (c) { note(key, true, n); return c; }
    }
    note(key, false, "class not found");
    return nullptr;
}

// field lookup: name/sig pairs tried in order (candidate list)
static jfieldID f_sig(JNIEnv* e, jclass c, const char* key,
                      std::initializer_list<const char*> p) {
    if (!c) { note(key, false, "(no owner class)"); return nullptr; }
    const size_t n = p.size();
    auto it = p.begin();
    for (size_t i = 0; i + 1 < n; i += 2) {
        jfieldID id = e->GetFieldID(e, c, it[i], it[i + 1]);
        if (!id) { jvm::check_exc(e, "GetFieldID"); continue; }
        note(key, true, it[i]);
        return id;
    }
    note(key, false, "field not found");
    return nullptr;
}

// method lookup: name/sig pairs tried in order (candidate list)
static jmethodID m_sig(JNIEnv* e, jclass c, const char* key,
                       std::initializer_list<const char*> p) {
    if (!c) { note(key, false, "(no owner class)"); return nullptr; }
    const size_t n = p.size();
    auto it = p.begin();
    for (size_t i = 0; i + 1 < n; i += 2) {
        jmethodID id = e->GetMethodID(e, c, it[i], it[i + 1]);
        if (!id) { jvm::check_exc(e, "GetMethodID"); continue; }
        note(key, true, it[i]);
        return id;
    }
    note(key, false, "method not found");
    return nullptr;
}

void resolve_all() {
    JNIEnv* e = jvm::env();
    if (!e) { clog::error(TAG, "no env - mappings unresolved"); return; }
    g_report.clear();

    // =============== classes ===============
    M.mc          = find_first(e, "mc", { "net/minecraft/client/MinecraftClient", "class_310" });
    M.gameOptions = find_first(e, "gameOptions", { "net/minecraft/client/option/GameOptions", "class_315" });
    M.world       = find_first(e, "world", { "net/minecraft/client/world/ClientWorld", "class_638" });
    M.entity      = find_first(e, "entity", { "net/minecraft/entity/Entity", "class_1297" });
    M.living      = find_first(e, "living", { "net/minecraft/entity/LivingEntity", "class_1309" });
    M.player      = find_first(e, "player", { "net/minecraft/entity/player/PlayerEntity", "class_1657" });
    M.otherPlayer = find_first(e, "otherPlayer", { "net/minecraft/client/network/OtherClientPlayerEntity", "class_742" });
    M.itemEntity  = find_first(e, "itemEntity", { "net/minecraft/entity/ItemEntity", "class_1545" });
    M.blockEntity = find_first(e, "blockEntity", { "net/minecraft/block/entity/BlockEntity", "class_2586" });
    M.chestBE     = find_first(e, "chestBE", { "net/minecraft/block/entity/ChestBlockEntity", "class_2595" });
    M.box         = find_first(e, "box", { "net/minecraft/util/math/Box", "class_238" });
    M.abilities   = find_first(e, "abilities", { "net/minecraft/entity/player/PlayerAbilities", "class_2668" });
    M.blockPos    = find_first(e, "blockPos", { "net/minecraft/util/math/BlockPos", "class_2338" });
    M.vec3        = find_first(e, "vec3", { "net/minecraft/util/math/Vec3d", "class_243" });

    // =============== MinecraftClient ===============
    M.mc_getInstance = m_sig(e, M.mc, "mc.getInstance", {
        "method_1551", "()Lnet/minecraft/client/MinecraftClient;",
        "method_1551", "()Lclass_310;" });
    M.mc_player = f_sig(e, M.mc, "mc.player", {
        "field_1724", "Lnet/minecraft/client/network/ClientPlayerEntity;",
        "field_1724", "Lclass_746;",
        "field_1724", "Lnet/minecraft/entity/player/PlayerEntity;" });
    M.mc_world = f_sig(e, M.mc, "mc.world", {
        "field_1687", "Lnet/minecraft/client/world/ClientWorld;",
        "field_1687", "Lclass_638;",
        "field_1687", "Lnet/minecraft/world/World;" });
    M.mc_options = f_sig(e, M.mc, "mc.options", {
        "field_1774", "Lnet/minecraft/client/option/GameOptions;",
        "field_1774", "Lclass_315;" });
    M.mc_doAttack = m_sig(e, M.mc, "mc.doAttack", {
        "method_1583", "()Z" });

    // =============== GameOptions (legacy double fields + SimpleOption getters) ===
    M.opt_gamma = f_sig(e, M.gameOptions, "opt.gamma", {
        "field_1843", "D" });
    M.opt_getGamma = m_sig(e, M.gameOptions, "opt.getGamma", {
        "method_41723", "()Lnet/minecraft/class_7172;",
        "method_41723", "()Lclass_7172;",
        "method_42436", "()Lnet/minecraft/class_7172;",
        "method_42436", "()Lclass_7172;" });
    M.opt_fov = f_sig(e, M.gameOptions, "opt.fov", {
        "field_2966", "D" });
    M.opt_getFov = m_sig(e, M.gameOptions, "opt.getFov", {
        "method_42436", "()Lnet/minecraft/class_7172;",
        "method_42436", "()Lclass_7172;",
        "method_41723", "()Lnet/minecraft/class_7172;",
        "method_41723", "()Lclass_7172;" });

    // =============== Entity ===============
    M.e_getX = m_sig(e, M.entity, "e.getX", { "method_19538", "()D" });
    M.e_getY = m_sig(e, M.entity, "e.getY", { "method_19539", "()D" });
    M.e_getZ = m_sig(e, M.entity, "e.getZ", { "method_19540", "()D" });
    M.e_getBBox = m_sig(e, M.entity, "e.getBBox", { "method_5829", "()Lnet/minecraft/util/math/Box;", "method_5829", "()Lclass_238;" });
    M.e_isAlive = m_sig(e, M.entity, "e.isAlive", { "method_5705", "()Z" });
    M.e_setSprinting = m_sig(e, M.entity, "e.setSprinting", { "method_5729", "(Z)V" });
    M.e_getVelocity = m_sig(e, M.entity, "e.getVelocity", { "method_5744", "()Lnet/minecraft/util/math/Vec3d;", "method_5744", "()Lclass_243;" });
    M.e_setVelocityXYZ = m_sig(e, M.entity, "e.setVelocityXYZ", {
        "method_5735", "(DDD)V",       // setVelocity(double,double,double)
        "method_18803", "(DDD)V" });
    M.e_getYaw = m_sig(e, M.entity, "e.getYaw", { "method_36455", "()F", "method_43079", "()F" });
    M.e_getPitch = m_sig(e, M.entity, "e.getPitch", { "method_36456", "()F", "method_43080", "()F" });
    M.e_setYaw = m_sig(e, M.entity, "e.setYaw", { "method_36457", "(F)V" });
    M.e_setPitch = m_sig(e, M.entity, "e.setPitch", { "method_36458", "(F)V" });
    M.e_yawF = f_sig(e, M.entity, "e.yawF", { "field_6036", "F" });
    M.e_pitchF = f_sig(e, M.entity, "e.pitchF", { "field_6037", "F" });
    M.e_fallDistance = f_sig(e, M.entity, "e.fallDistance", {
        "field_1323", "D",             // 1.21.x float->double candidates
        "field_1312", "D",
        "field_1323", "F" });

    // =============== Box ===============
    M.box_minX = f_sig(e, M.box, "box.minX", { "field_1324", "D" });
    M.box_minY = f_sig(e, M.box, "box.minY", { "field_1325", "D" });
    M.box_minZ = f_sig(e, M.box, "box.minZ", { "field_1326", "D" });
    M.box_maxX = f_sig(e, M.box, "box.maxX", { "field_1327", "D" });
    M.box_maxY = f_sig(e, M.box, "box.maxY", { "field_1328", "D" });
    M.box_maxZ = f_sig(e, M.box, "box.maxZ", { "field_1329", "D" });

    // =============== LivingEntity ===============
    M.l_getHealth = m_sig(e, M.living, "l.getHealth", { "method_5442", "()F" });
    M.l_hurtTime = f_sig(e, M.living, "l.hurtTime", { "field_6009", "I" });

    // =============== PlayerEntity ===============
    M.p_getAbilities = m_sig(e, M.player, "p.getAbilities", {
        "method_37303", "()Lnet/minecraft/entity/player/PlayerAbilities;",
        "method_37303", "()Lclass_2668;" });
    M.p_abilities = f_sig(e, M.player, "p.abilities", {
        "field_7508", "Lnet/minecraft/entity/player/PlayerAbilities;",
        "field_7508", "Lclass_2668;" });

    // =============== PlayerAbilities ===============
    M.ab_flying      = f_sig(e, M.abilities, "ab.flying",      { "field_6809", "Z" });
    M.ab_allowFlying = f_sig(e, M.abilities, "ab.allowFlying", { "field_6812", "Z" });
    M.ab_walkSpeed   = f_sig(e, M.abilities, "ab.walkSpeed",   { "field_6813", "F" });
    M.ab_flySpeed    = f_sig(e, M.abilities, "ab.flySpeed",    { "field_6812", "F", "field_6813", "F" });

    // =============== ClientWorld ===============
    M.w_getEntities = m_sig(e, M.world, "w.getEntities", {
        "method_18112", "()Ljava/lang/Iterable;",
        "method_1823",  "()Ljava/lang/Iterable;",
        "method_3159",  "()Ljava/lang/Iterable;" });
    M.w_getBlockEntities = m_sig(e, M.world, "w.getBlockEntities", {
        "method_46595", "()Ljava/util/Collection;",
        "method_46595", "()Ljava/lang/Iterable;",
        "method_18233", "()Ljava/util/Collection;",
        "method_18233", "()Ljava/lang/Iterable;" });

    // world-level setters are client-side only here (singleplayer friendly):
    // we accept they may miss; features degrade.
    M.w_setTime = m_sig(e, M.world, "w.setTime", {
        "method_29091", "(J)V",
        "method_29090", "(J)V",
        "method_27909", "(J)V",
        "method_27909", "(J)V" });
    M.w_setRain = m_sig(e, M.world, "w.setRain", {
        "method_27908", "(F)V",
        "method_29089", "(F)V",
        "method_27910", "(F)V" });

    // =============== BlockEntity / BlockPos ===============
    M.be_getPos = m_sig(e, M.blockEntity, "be.getPos", {
        "method_4992", "()Lnet/minecraft/util/math/BlockPos;",
        "method_4992", "()Lclass_2338;" });
    M.bp_getX = m_sig(e, M.blockPos, "bp.getX", { "method_10260", "()I" });
    M.bp_getY = m_sig(e, M.blockPos, "bp.getY", { "method_10261", "()I" });
    M.bp_getZ = m_sig(e, M.blockPos, "bp.getZ", { "method_10262", "()I" });

    // =============== Vec3d ===============
    M.v_x = f_sig(e, M.vec3, "v.x", { "field_1351", "D" });
    M.v_y = f_sig(e, M.vec3, "v.y", { "field_1352", "D" });
    M.v_z = f_sig(e, M.vec3, "v.z", { "field_1353", "D" });

    // remove helpers that resolved against nullptr classes
    clog::info(TAG, "mapping pass complete: %d entries", (int)g_report.size());
}

} // namespace mappings

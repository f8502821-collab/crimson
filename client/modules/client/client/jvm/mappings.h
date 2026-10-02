#pragma once
// Central JNI mapping registry for Fabric 1.21.11 (intermediary names).
// EVERY class/field/method the client touches is declared here and resolved
// once at load. A miss never crashes: it disables the feature using it.
#include "client/jvm/jni_min.h"
#include <string>
#include <vector>

namespace mappings {

struct All {
    // --- classes ------------------------------------------------------------
    jclass mc{};              // class_310  MinecraftClient
    jclass gameOptions{};     // class_315  GameOptions
    jclass world{};           // class_638  ClientWorld
    jclass entity{};          // class_1297 Entity
    jclass living{};          // class_1309 LivingEntity
    jclass player{};          // class_1657 PlayerEntity
    jclass otherPlayer{};     // class_742  OtherClientPlayerEntity
    jclass itemEntity{};      // class_1545 ItemEntity
    jclass blockEntity{};     // class_2586 BlockEntity
    jclass chestBE{};         // class_2595 ChestBlockEntity
    jclass box{};             // class_238  Box
    jclass abilities{};       // class_2668 PlayerAbilities
    jclass blockPos{};        // class_2338 BlockPos (methods live on Vec3i)
    jclass vec3{};            // class_243  Vec3d

    // --- MinecraftClient ----------------------------------------------------
    jmethodID mc_getInstance{};   // method_1551
    jfieldID  mc_player{};        // field_1724
    jfieldID  mc_world{};         // field_1687
    jfieldID  mc_options{};       // field_1774
    jmethodID mc_doAttack{};      // method_1583 ()Z

    // --- GameOptions --------------------------------------------------------
    jfieldID  opt_gamma{};        // legacy field_1843 D (candidates, may miss)
    jmethodID opt_getGamma{};     // SimpleOption-returning getter (candidates)
    jfieldID  opt_fov{};          // legacy field_2966 D (candidates)
    jmethodID opt_getFov{};       // SimpleOption-returning getter (candidates)

    // --- Entity -------------------------------------------------------------
    jmethodID e_getX{};           // method_19538 D
    jmethodID e_getY{};           // method_19539 D
    jmethodID e_getZ{};           // method_19540 D
    jmethodID e_getBBox{};        // method_5829 ()LBox;
    jmethodID e_isAlive{};        // method_5705 ()Z
    jmethodID e_setSprinting{};   // method_5729 (Z)V
    jmethodID e_getVelocity{};    // method_5744 ()LVec3d;
    jmethodID e_setVelocityXYZ{}; // (DDD)V (candidates)
    jmethodID e_getYaw{};         // ()F (candidates)
    jmethodID e_getPitch{};       // ()F (candidates)
    jmethodID e_setYaw{};         // (F)V (candidates)
    jmethodID e_setPitch{};       // (F)V (candidates)
    jfieldID  e_yawF{};           // legacy float field (candidates)
    jfieldID  e_pitchF{};         // legacy float field (candidates)
    jfieldID  e_fallDistance{};   // D (candidates)

    // --- Box ----------------------------------------------------------------
    jfieldID box_minX{}; jfieldID box_minY{}; jfieldID box_minZ{};
    jfieldID box_maxX{}; jfieldID box_maxY{}; jfieldID box_maxZ{};

    // --- LivingEntity -------------------------------------------------------
    jmethodID l_getHealth{};      // method_5442 ()F
    jfieldID  l_hurtTime{};       // I (candidates)

    // --- PlayerEntity -------------------------------------------------------
    jmethodID p_getAbilities{};   // ()LPlayerAbilities; (candidates)
    jfieldID  p_abilities{};      // legacy field (fallback)

    // --- PlayerAbilities ----------------------------------------------------
    jfieldID ab_flying{};         // Z (candidates)
    jfieldID ab_allowFlying{};    // Z (candidates)
    jfieldID ab_walkSpeed{};      // F (candidates)
    jfieldID ab_flySpeed{};       // F (candidates)

    // --- ClientWorld --------------------------------------------------------
    jmethodID w_getEntities{};      // ()Ljava/lang/Iterable; (candidates)
    jmethodID w_getBlockEntities{}; // ()Ljava/util/Collection; (candidates)
    jmethodID w_setTime{};          // (J)V (candidates, low confidence)
    jmethodID w_setRain{};          // (Z)V / similar (candidates, low confidence)

    // --- BlockEntity / BlockPos ---------------------------------------------
    jmethodID be_getPos{};        // ()LBlockPos; (candidates)
    jmethodID bp_getX{};          // method_10260 ()I
    jmethodID bp_getY{};          // method_10261 ()I
    jmethodID bp_getZ{};          // method_10262 ()I

    // --- Vec3d --------------------------------------------------------------
    jfieldID v_x{}; jfieldID v_y{}; jfieldID v_z{};
};

extern All M;

// Resolve everything once; logs a per-key report ("key OK/MISS detail").
void resolve_all();

// Did <key> resolve? (keys are the exact logical names used in resolve_all)
bool ok(const char* key);

// Full report lines for the ClickGUI debug tab.
const std::vector<std::string>& report();

} // namespace mappings

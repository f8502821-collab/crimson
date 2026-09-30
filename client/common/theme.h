#pragma once
// CRIMSON theme tokens: shared palette for injector + client.
#include <imgui.h>
#include <cmath>
#include <cstdint>

namespace theme {

// im32 colors are ABGR (0xAABBGGRR).
constexpr ImU32 ACCENT      = IM_COL32(255, 30, 60, 255);   // #FF1E3C crimson
constexpr ImU32 ACCENT_HOT  = IM_COL32(255, 96, 48, 255);   // #FF6030 ember
constexpr ImU32 ACCENT_DEEP = IM_COL32(122, 10, 32, 255);   // #7A0A20

constexpr ImU32 BG_DEEP     = IM_COL32(8, 7, 11, 255);      // near-black base
constexpr ImU32 BG_GLASS    = IM_COL32(16, 13, 20, 214);    // glass panel fill
constexpr ImU32 BG_GLASS_2  = IM_COL32(24, 18, 28, 232);

constexpr ImU32 TEXT_HI     = IM_COL32(245, 240, 248, 255);
constexpr ImU32 TEXT_MID    = IM_COL32(168, 158, 178, 255);
constexpr ImU32 TEXT_LOW    = IM_COL32(96, 88, 108, 255);

constexpr ImU32 OK          = IM_COL32(60, 220, 130, 255);
constexpr ImU32 WARN        = IM_COL32(255, 176, 48, 255);
constexpr ImU32 BAD         = IM_COL32(255, 60, 60, 255);

constexpr float CORNER      = 10.f;   // base corner radius
constexpr float CORNER_SM   = 6.f;

constexpr const char* NAME = "CRIMSON";
constexpr const char* VERSION = "1.0";
constexpr const char* MC_VERSION = "1.21.11";

// Helpers to derive alpha/lerped variants at runtime.
inline ImU32 with_alpha(ImU32 c, float a01) {
    const int a = (int)(a01 * 255.f);
    return (c & 0x00FFFFFFu) | (ImU32)(a << 24);
}
inline ImU32 mix(ImU32 a, ImU32 b, float t) {
    const float ar = (float)(a & 0xFF), ag = (float)((a >> 8) & 0xFF), ab = (float)((a >> 16) & 0xFF);
    const float br = (float)(b & 0xFF), bg = (float)((b >> 8) & 0xFF), bb = (float)((b >> 16) & 0xFF);
    const float u = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
    return IM_COL32((int)(ar + (br - ar) * u),
                    (int)(ag + (bg - ag) * u),
                    (int)(ab + (bb - ab) * u),
                    255);
}
// Pulse value in [0,1] from a global time.
inline float pulse(float t, float period = 2.2f, float phase = 0.f) {
    const float x = fmodf(t * (1.f / period) + phase, 1.f);
    return 0.5f - 0.5f * cosf(x * 6.2831853f);
}

} // namespace theme

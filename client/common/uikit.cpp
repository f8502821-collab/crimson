#include "common/uikit.h"
#include "common/theme.h"
#include "common/easing.h"
#include <cmath>
#include <algorithm>

namespace uikit {

static float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

void glow_rect(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
               float rounding, ImU32 accent, float glow_alpha01, float fill_alpha01) {
    // 3 expanding translucent strokes behind the fill
    for (int i = 3; i >= 1; --i) {
        const float expand = 2.f + 4.f * (float)i;
        const float a = glow_alpha01 * (0.16f / (float)i);
        dl->AddRect(mn - ImVec2(expand, expand), mx + ImVec2(expand, expand),
                    theme::with_alpha(accent, a), rounding + expand, 0, 2.4f);
    }
    if (fill_alpha01 > 0.001f) {
        dl->AddRectFilled(mn, mx, theme::with_alpha(accent, fill_alpha01 * 0.10f), rounding);
    }
}

void gradient_border(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
                     float rounding, float t, ImU32 accent,
                     float thickness, float trail) {
    // approximate a traveling arc of color by stroking the rect N segments and
    // coloring each by its angular distance behind the current head position
    constexpr int N = 48;
    ImVec2 prev(mx.x - rounding, mn.y);
    for (int i = 1; i <= N; ++i) {
        const float u = (float)i / (float)N;
        const float d = std::fmod(u - t + 1.f, 1.f);
        const float k = clamp01(1.f - d / trail);
        if (k <= 0.02f) {
            prev = ImVec2(mn.x + (mx.x - mn.x) * u, mn.y);
            continue;
        }
        ImVec2 cur(mn.x + (mx.x - mn.x) * u, mn.y);
        dl->AddLine(prev, cur, theme::with_alpha(accent, 0.85f * k * k), thickness);
        prev = cur;
    }
    // vertical comet-tail on the right edge
    const float fall = (mx.y - mn.y) * trail;
    for (int s = 0; s < 6; ++s) {
        const float f0 = (float)s / 6.f, f1 = (float)(s + 1) / 6.f;
        const float k = clamp01(1.f - f0);
        dl->AddLine(ImVec2(mx.x - 1.f, mn.y + fall * f0),
                    ImVec2(mx.x - 1.f, mn.y + fall * f1),
                    theme::with_alpha(accent, 0.55f * k * k), thickness);
    }
}

void chamfer_rect(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
                  float cut, ImU32 fill, ImU32 line, float line_alpha01) {
    const float c = cut;
    const ImVec2 poly[8] = {
        { mn.x + c, mn.y }, { mx.x - c, mn.y },
        { mx.x, mn.y + c }, { mx.x, mx.y - c },
        { mx.x - c, mx.y }, { mn.x + c, mx.y },
        { mn.x, mx.y - c }, { mn.x, mn.y + c }
    };
    dl->AddConvexPolyFilled(poly, 8, fill);

    for (int i = 0; i < 8; ++i) {
        dl->AddLine(poly[i], poly[(i + 1) % 8], theme::with_alpha(line, line_alpha01), 1.2f);
    }
}

void hatch(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float spacing,
           ImU32 col, float phase) {
    const float w = mx.x - mn.x, h = mx.y - mn.y;
    const float diag = w + h;
    float off = std::fmod(phase, spacing);
    for (float d = -diag; d < diag; d += spacing) {
        const float x0 = mn.x + d + off;
        dl->AddLine(ImVec2(x0, mx.y), ImVec2(x0 + h, mn.y), col, 1.0f);
    }
}

void grad_rect_v(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
                 ImU32 top, ImU32 mid, ImU32 bottom) {
    const float midY = (mn.y + mx.y) * 0.5f;
    dl->AddRectFilledMultiColor(mn, ImVec2(mx.x, midY), top, mid, mid, top);
    dl->AddRectFilledMultiColor(ImVec2(mn.x, midY), mx, mid, bottom, bottom, mid);
}

void shimmer(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float rounding,
             float a01, ImU32 accent) {
    const float w = mx.x - mn.x;
    const float cw = w * 0.22f;
    float cx = mn.x + (w + cw) * a01 - cw;
    cx = cx < mn.x ? mn.x : (cx > mx.x - cw ? mx.x - cw : cx);
    // band with soft edges via 3 overlapping translucent columns
    for (int i = 0; i < 3; ++i) {
        const float pad = (float)i * cw * 0.28f;
        const float a = 0.10f * (1.f - 0.25f * (float)i);
        dl->AddRectFilled(ImVec2(cx + pad, mn.y), ImVec2(cx + cw - pad, mx.y),
                          theme::with_alpha(accent, a), rounding);
    }
}

void ripple(ImDrawList* dl, const ImVec2& center, float a01, ImU32 accent, float max_radius) {
    const float r = max_radius * ez::ease_out_cubic(clamp01(a01));
    const float a = (1.f - clamp01(a01));
    dl->AddCircle(center, r, theme::with_alpha(accent, 0.85f * a), 0, 2.0f);
    dl->AddCircle(center, r * 0.62f, theme::with_alpha(accent, 0.4f * a), 0, 1.4f);
}

void dust(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float t,
          int count, ImU32 accent, uint32_t seed) {
    uint32_t s = seed * 747796405u + 2891336453u;
    auto rnd = [&s]() { s ^= s >> 13; s ^= s << 17; s ^= s >> 5; return s; };
    const float w = mx.x - mn.x, h = mx.y - mn.y;
    for (int i = 0; i < count; ++i) {
        const float u0 = (float)(rnd() & 0xFFFF) / 65535.f;
        const float u1 = (float)(rnd() & 0xFFFF) / 65535.f;
        const float spd = 6.f + 22.f * u1;
        float x = mn.x + w * u0;
        float y = mx.y - std::fmod(u1 * h + t * spd, h);
        const float tw = 0.5f - 0.5f * std::cos((t * (1.5f + u0) + u1 * 6.28f));
        dl->AddCircleFilled(ImVec2(x, y), 1.1f + 1.4f * u1,
                            theme::with_alpha(accent, 0.06f + 0.22f * tw));
    }
}

void vignette(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float strength01) {
    const float fw = (mx.x - mn.x) * 0.22f * strength01;
    const float fh = (mx.y - mn.y) * 0.26f * strength01;
    dl->AddRectFilledMultiColor(mn, ImVec2(mn.x + fw, mx.y),
                                IM_COL32(0, 0, 0, 150), IM_COL32(0, 0, 0, 0),
                                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 150));
    dl->AddRectFilledMultiColor(ImVec2(mx.x - fw, mn.y), mx,
                                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 150),
                                IM_COL32(0, 0, 0, 150), IM_COL32(0, 0, 0, 0));
    dl->AddRectFilledMultiColor(ImVec2(mn.x, mn.y), ImVec2(mx.x, mn.y + fh),
                                IM_COL32(0, 0, 0, 150), IM_COL32(0, 0, 0, 150),
                                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0));
    dl->AddRectFilledMultiColor(ImVec2(mn.x, mx.y - fh), mx,
                                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                                IM_COL32(0, 0, 0, 150), IM_COL32(0, 0, 0, 150));
}

float hover_glow(const ImVec2& mn, const ImVec2& mx, float speed) {
    ImGuiIO& io = ImGui::GetIO();
    static float g = 0.f;
    const bool hov = io.MousePos.x >= mn.x && io.MousePos.x <= mx.x &&
                     io.MousePos.y >= mn.y && io.MousePos.y <= mx.y;
    g = ez::damp(g, hov ? 1.f : 0.f, speed, io.DeltaTime);
    return g;
}

} // namespace uikit

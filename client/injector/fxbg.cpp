#include "injector/fxbg.h"
#include "common/theme.h"
#include "common/uikit.h"
#include "common/easing.h"
#include <cmath>

namespace fxbg {

static float wobble(float t, float speed, float phase) {
    return 0.5f + 0.5f * std::sin(t * speed + phase);
}

void draw(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, double time) {
    const float t = (float)time;
    const float W = mx.x - mn.x, H = mx.y - mn.y;

    // base: deep vertical gradient
    uikit::grad_rect_v(dl, mn, mx,
        IM_COL32(10, 6, 12, 255),
        IM_COL32(14, 8, 18, 255),
        IM_COL32(6, 4, 9, 255));

    // aurora blobs: big soft radial-ish glows drifting slowly
    struct Blob { float bx, by, r; ImU32 col; float spd, ph; };
    const Blob blobs[3] = {
        { 0.24f, 0.30f, 0.55f, IM_COL32(122, 10, 32, 255), 0.11f, 0.0f },
        { 0.78f, 0.66f, 0.62f, IM_COL32(70, 6, 40, 255),   0.09f, 2.1f },
        { 0.50f, 0.92f, 0.45f, IM_COL32(40, 4, 26, 255),   0.13f, 4.4f },
    };
    for (const auto& b : blobs) {
        const float drift = wobble(t, b.spd, b.ph);
        const ImVec2 c(
            mn.x + W * (b.bx + 0.05f * (drift - 0.5f)),
            mn.y + H * (b.by + 0.04f * (wobble(t, b.spd * 1.3f, b.ph + 1.f) - 0.5f)));
        const float R = W * b.r * (0.9f + 0.15f * drift);
        // layered circles simulate a soft radial falloff
        for (int i = 8; i >= 1; --i) {
            const float rr = R * (float)i / 8.f;
            const float a = 0.020f * (1.f - (float)i / 9.f);
            dl->AddCircleFilled(c, rr, theme::with_alpha(b.col, a), 40);
        }
    }

    // sweeping light waves: sine "curtains" of translucent crimson
    for (int k = 0; k < 3; ++k) {
        const float alpha = 0.05f - 0.012f * k;
        const float amp = H * (0.05f + 0.025f * k);
        const float base = mn.y + H * (0.42f + 0.16f * k);
        const float spd = 0.35f + 0.12f * k;
        const float ph = t * spd + k * 1.7f;
        for (float x = 0.f; x <= W; x += 14.f) {
            const float y = base + amp * std::sin(x * 0.004f + ph)
                                  + amp * 0.4f * std::sin(x * 0.011f - ph * 0.7f);
            dl->AddLine(ImVec2(mn.x + x, y), ImVec2(mn.x + x + 15.f, y),
                        theme::with_alpha(theme::ACCENT, alpha), 26.f);
        }
    }

    // particle dust
    uikit::dust(dl, mn, mx, t, 64, theme::ACCENT, 1337u);

    // vignette + top hatch strip for texture
    uikit::vignette(dl, mn, mx, 0.9f);
    dl->PushClipRect(mn, mx, true);
    uikit::hatch(dl, ImVec2(mn.x, mn.y), ImVec2(mx.x, mn.y + 64.f), 7.f,
                 theme::with_alpha(theme::ACCENT, 0.05f), t * 8.f);
    dl->PopClipRect();
}

} // namespace fxbg

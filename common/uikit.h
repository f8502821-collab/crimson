#pragma once
// CRIMSON DrawList FX kit: layered glow, marching gradient borders, chamfered
// panels, shimmer sweeps, ripples, particles. Shared by injector + client.
#include <imgui.h>
#include <cstdint>

namespace uikit {

// --- primitives -------------------------------------------------------------

// Rounded filled rect with a 3-layer outer glow in `accent`.
void glow_rect(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
               float rounding, ImU32 accent, float glow_alpha01, float fill_alpha01);

// marching gradient border (animated dash of color traveling around the rect)
void gradient_border(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
                     float rounding, float t, ImU32 accent,
                     float thickness = 1.4f, float trail = 0.38f);

// panel with cut corners (chamfer) and optional 1px inner line
void chamfer_rect(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
                  float cut, ImU32 fill, ImU32 line, float line_alpha01 = 1.f);

// diagonal hatch stripes clipped to a rect (header texture)
void hatch(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float spacing,
           ImU32 col, float phase = 0.f);

// vertical/horizontal multi-stop gradient rect (3 stops)
void grad_rect_v(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx,
                 ImU32 top, ImU32 mid, ImU32 bottom);

// --- effects ----------------------------------------------------------------

// moving specular highlight across the rect; a01 = sweep position 0..1 (loops)
void shimmer(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float rounding,
             float a01, ImU32 accent);

// expanding ring from click point; a01 = progress 0..1
void ripple(ImDrawList* dl, const ImVec2& center, float a01, ImU32 accent,
            float max_radius = 46.f);

// drifting dust particles inside a rect; deterministic per seed
void dust(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float t,
          int count, ImU32 accent, uint32_t seed = 7u);

// soft vignette (4 gradient flanges), col usually black
void vignette(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, float strength01);

// --- interaction ------------------------------------------------------------

// hovered = mouse inside rect; returns pulse alpha for hover glow
float hover_glow(const ImVec2& mn, const ImVec2& mx, float speed = 8.f);

} // namespace uikit

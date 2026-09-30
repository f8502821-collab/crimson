#pragma once
// In-game GUI theming over the shared palette + fonts.
namespace gui_theme {

void apply();          // push imgui style once at startup
void set_accent_from_hue(float hue01);   // rotate the crimson hue

} // namespace gui_theme

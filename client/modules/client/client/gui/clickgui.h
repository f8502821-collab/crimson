#pragma once
// ClickGUI: rail + floating panels, search, FX/system tab, mapping debug.
namespace clickgui {

void init();
void shutdown();
void toggle();
bool is_open();

float fx_amount();              // 0..1 post-fx intensity (user setting)
double time_since_close();      // seconds since GUI closed (for fade-out)

void draw(double now);

} // namespace clickgui

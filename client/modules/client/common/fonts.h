#pragma once
// Shared font registry: both binaries register their faces here at startup
// and refer to them by enum instead of raw ImFont*.
#include <imgui.h>

namespace fonts {

enum Id : int {
    UI = 0,    // display face (Gruppo) - titles, buttons, branding
    MONO,      // JetBrains Mono - numbers, lists, logs
    COUNT
};

// Must be called after ImGui context creation and before the first frame.
void build();

inline ImFont* get(Id id) {
    ImGuiIO& io = ImGui::GetIO();
    const int idx = (int)id;
    return io.Fonts->Fonts[idx];
}

inline ImFont* ui()  { return get(UI); }
inline ImFont* mono(){ return get(MONO); }

} // namespace fonts

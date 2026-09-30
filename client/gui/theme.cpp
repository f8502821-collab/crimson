#include "client/gui/theme.h"
#include "common/theme.h"
#include <imgui.h>

namespace gui_theme {

void apply() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 0.f;
    s.FrameRounding = 4.f;
    s.GrabRounding = 4.f;
    s.WindowBorderSize = 0.f;
    s.FrameBorderSize = 0.f;
    s.ScrollbarSize = 10.f;
    s.ScrollbarRounding = 5.f;
    s.WindowPadding = ImVec2(10, 10);
    s.FramePadding = ImVec2(8, 5);
    s.ItemSpacing = ImVec2(8, 6);

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.045f, 0.07f, 0.94f);
    c[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.04f, 0.065f, 0.85f);
    c[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.04f, 0.065f, 0.97f);
    c[ImGuiCol_Border] = ImVec4(0.55f, 0.06f, 0.14f, 0.0f);
    c[ImGuiCol_Text] = ImVec4(0.96f, 0.94f, 0.97f, 1.f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.42f, 0.5f, 1.f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.025f, 0.04f, 0.6f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.06f, 0.14f, 0.6f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.8f, 0.1f, 0.2f, 0.8f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.f, 0.12f, 0.24f, 1.f);
    c[ImGuiCol_SliderGrab] = ImVec4(1.f, 0.12f, 0.24f, 0.85f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(1.f, 0.35f, 0.25f, 1.f);
    c[ImGuiCol_CheckMark] = ImVec4(1.f, 0.12f, 0.24f, 1.f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(1.f, 0.12f, 0.24f, 0.35f);
    c[ImGuiCol_Header] = ImVec4(0.55f, 0.06f, 0.14f, 0.4f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.08f, 0.17f, 0.6f);
    c[ImGuiCol_HeaderActive] = ImVec4(1.f, 0.12f, 0.24f, 0.7f);
    c[ImGuiCol_Button] = ImVec4(0.55f, 0.06f, 0.14f, 0.55f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.1f, 0.2f, 0.8f);
    c[ImGuiCol_ButtonActive] = ImVec4(1.f, 0.12f, 0.24f, 1.f);
    c[ImGuiCol_ResizeGrip] = ImVec4(0.f, 0.f, 0.f, 0.f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(1.f, 0.12f, 0.24f, 0.4f);
    c[ImGuiCol_ResizeGripActive] = ImVec4(1.f, 0.12f, 0.24f, 0.8f);
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f, 0.f, 0.f, 0.6f);
}

void set_accent_from_hue(float hue01) {
    // rotate the accent within ImGui's color palette
    ImVec4 accent;
    ImGui::ColorConvertHSVtoRGB(hue01, 0.82f, 1.f, accent.x, accent.y, accent.z);
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_CheckMark] = accent;
    s.Colors[ImGuiCol_SliderGrab] = accent;
    s.Colors[ImGuiCol_SliderGrabActive] = accent;
    s.Colors[ImGuiCol_ButtonActive] = accent;
    s.Colors[ImGuiCol_HeaderActive] = accent;
    s.Colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
}

} // namespace gui_theme

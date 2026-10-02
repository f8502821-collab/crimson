#include "client/gui/widgets.h"
#include "common/theme.h"
#include "common/uikit.h"
#include "common/easing.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cmath>

namespace widgets {

// animated per-id state
struct Anim { float t = 0.f; };
static Anim* anim_for(const char* id) {
    static const int N = 512;
    static Anim pool[N];
    static const char* ids[N] = {};
    static int next = 0;
    unsigned h = 2166136261u;
    for (const char* p = id; *p; ++p) h = (h ^ (unsigned)*p) * 16777619u;
    const int slot = (int)(h % N);
    if (ids[slot] != id) { ids[slot] = id; pool[slot] = Anim{}; }
    return &pool[slot];
}

bool toggle(const char* id, bool* v) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float h = 20.f, w = 38.f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 mx(p.x + w, p.y + h);
    bool clicked = false;

    ImGui::InvisibleButton(id, ImVec2(w, h));
    if (ImGui::IsItemClicked()) { *v = !*v; clicked = true; }

    Anim* a = anim_for(id);
    const float target = *v ? 1.f : 0.f;
    a->t = ez::damp(a->t, target, 14.f, ImGui::GetIO().DeltaTime);

    const float rad = h * 0.5f;
    const ImU32 off = IM_COL32(50, 44, 58, 255);
    const ImU32 on  = theme::ACCENT;

    if (*v) uikit::glow_rect(dl, p, mx, rad, on, 0.35f * a->t, 0.f);
    dl->AddRectFilled(p, mx, *v ? on : off, rad);
    // knob
    const float kx = p.x + rad + (w - h) * a->t;
    dl->AddCircleFilled(ImVec2(kx, p.y + rad), rad - 3.f, theme::TEXT_HI);
    return clicked;
}

bool slider(const char* id, float* v, float mn, float mx, const char* fmt) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float h = 14.f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    ImVec2 mx(p.x + w, p.y + h);

    ImGui::InvisibleButton(id, ImVec2(w, h));
    if (ImGui::IsItemActive()) {
        const float t = (ImGui::GetIO().MousePos.x - p.x) / w;
        *v = mn + (mx - mn) * (t < 0.f ? 0.f : (t > 1.f ? 1.f : t));
    }
    const float t = (*v - mn) / (mx - mn);

    // track + fill + glow head
    dl->AddRectFilled(p, mx, IM_COL32(38, 32, 44, 255), h * 0.5f);
    if (t > 0.001f)
        dl->AddRectFilled(p, ImVec2(p.x + w * t, mx.y), theme::ACCENT, h * 0.5f);
    const ImVec2 knob(p.x + w * t, (p.y + mx.y) * 0.5f);
    dl->AddCircleFilled(knob, 5.f, theme::TEXT_HI);
    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
        dl->AddCircleFilled(knob, 9.f, theme::with_alpha(theme::ACCENT, 0.25f));

    // value text right-aligned above
    char val[32];
    snprintf(val, sizeof(val), fmt, *v);
    dl->AddText(ImVec2(mx.x - ImGui::CalcTextSize(val).x, p.y - 14.f), theme::TEXT_MID, val);
    return ImGui::IsItemActive();
}

bool chip(const char* label, bool active, bool* toggled) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    const float h = ImGui::GetTextLineHeight() + 10.f;
    ImVec2 mx(p.x + w, p.y + h);
    *toggled = false;

    ImGui::InvisibleButton(label, ImVec2(w, h));
    if (ImGui::IsItemClicked()) *toggled = true;

    if (active) {
        dl->AddRectFilled(p, mx, theme::with_alpha(theme::ACCENT, 0.14f), 6.f);
        dl->AddRect(p, mx, theme::with_alpha(theme::ACCENT, 0.55f), 6.f, 0, 1.2f);
    }    else if (ImGui::IsItemHovered()) {
        dl->AddRectFilled(p, mx, IM_COL32(64, 8, 16, 20), 6.f);
    }
    dl->AddCircleFilled(ImVec2(p.x + 9.f, (p.y + mx.y) * 0.5f), 2.5f,
                        active ? theme::ACCENT : theme::TEXT_LOW);
    dl->AddText(ImVec2(p.x + 18.f, p.y + 5.f), active ? theme::TEXT_HI : theme::TEXT_MID, label);
    return true;
}

bool keybind(const char* id, int* vk) {
    static const char* capturing = nullptr;
    ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = 70.f, h = 22.f;
    bool start = false;
    ImGui::InvisibleButton(id, ImVec2(w, h));
    if (ImGui::IsItemClicked()) { capturing = (capturing == id) ? nullptr : id; start = true; }

    const bool cap = capturing == id;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (cap) {
        uikit::gradient_border(dl, p, ImVec2(p.x + w, p.y + h), 4.f,
                               (float)ImGui::GetTime() * 0.9f, theme::ACCENT, 1.4f);
        dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), theme::with_alpha(theme::ACCENT, 0.08f), 4.f);
    } else {
        dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), IM_COL32(34, 28, 40, 255), 4.f);
        dl->AddRect(p, ImVec2(p.x + w, p.y + h), IM_COL32(70, 60, 80, 255), 4.f);
    }
    char buf[16];
    if (cap) {
        snprintf(buf, sizeof(buf), "...");
    } else if (!*vk) {
        snprintf(buf, sizeof(buf), "none");
    }
    // decode vk to a readable name
    if (!cap && *vk) {
        if (*vk >= 'A' && *vk <= 'Z') snprintf(buf, sizeof(buf), "%c", (char)*vk);
        else if (*vk >= '0' && *vk <= '9') snprintf(buf, sizeof(buf), "%c", (char)*vk);
        else if (*vk == VK_SHIFT) snprintf(buf, sizeof(buf), "SHIFT");
        else if (*vk == VK_CONTROL) snprintf(buf, sizeof(buf), "CTRL");
        else if (*vk == VK_MENU) snprintf(buf, sizeof(buf), "ALT");
        else if (*vk == VK_INSERT) snprintf(buf, sizeof(buf), "INS");
        else snprintf(buf, sizeof(buf), "0x%02X", *vk);
    }
    dl->AddText(ImVec2(p.x + (w - ImGui::CalcTextSize(buf).x) * 0.5f, p.y + 4.f),
                cap ? theme::ACCENT : theme::TEXT_MID, buf);
    return cap;
}

bool section(const char* label, bool* open) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    const float h = ImGui::GetTextLineHeight() + 8.f;
    ImGui::InvisibleButton(label, ImVec2(w, h));
    if (ImGui::IsItemClicked()) *open = !*open;
    const float a = *open ? 1.f : 0.f;
    dl->AddLine(ImVec2(p.x, p.y + h), ImVec2(p.x + w, p.y + h),
                theme::with_alpha(theme::ACCENT, 0.35f), 1.f);
    dl->AddTriangleFilled(ImVec2(p.x + w - 10.f + 5.f * (1.f - a), p.y + 6.f + 4.f * a),
                          ImVec2(p.x + w - 4.f - 5.f * (1.f - a), p.y + 6.f + 4.f * a),
                          ImVec2(p.x + w - 7.f, p.y + 12.f - 4.f * a),
                          theme::ACCENT);
    dl->AddText(ImVec2(p.x + 2.f, p.y + 3.f), theme::TEXT_HI, label);
    return *open;
}

// toasts
struct Toast { char msg[160]; double t0; };
static std::vector<Toast> g_toasts;

void toast(const char* msg) {
    Toast t{};
    snprintf(t.msg, sizeof(t.msg), "%s", msg);
    t.t0 = ImGui::GetTime();
    g_toasts.push_back(t);
    if (g_toasts.size() > 6) g_toasts.erase(g_toasts.begin());
}

void draw_toasts(double now) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    float y = W.y - 40.f;
    for (int i = (int)g_toasts.size() - 1; i >= 0; --i) {
        const float age = (float)(now - g_toasts[i].t0);
        if (age > 3.f) { g_toasts.erase(g_toasts.begin() + i); continue; }
        const float a = age < 2.5f ? 1.f : 1.f - (age - 2.5f) * 2.f;
        const ImVec2 ts = ImGui::CalcTextSize(g_toasts[i].msg);
        const ImVec2 mn(W.x - ts.x - 34.f, y - ts.y - 10.f);
        const ImVec2 mx(W.x - 16.f, y);
        dl->AddRectFilled(mn, mx, theme::with_alpha(theme::BG_GLASS_2, 0.92f * a), 6.f);
        dl->AddRect(mn, mx, theme::with_alpha(theme::ACCENT, 0.5f * a), 6.f);
        dl->AddCircleFilled(ImVec2(mn.x + 10.f, (mn.y + mx.y) * 0.5f), 2.5f,
                            theme::with_alpha(theme::ACCENT, a));
        dl->AddText(ImVec2(mn.x + 18.f, mn.y + 5.f), theme::with_alpha(theme::TEXT_HI, a),
                    g_toasts[i].msg);
        y = mn.y - 6.f;
    }
}

} // namespace widgets

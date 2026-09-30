#include "client/gui/clickgui.h"
#include "client/gui/widgets.h"
#include "client/gui/theme.h"
#include "client/modules/module_manager.h"
#include "client/jvm/mappings.h"
#include "client/loader.h"
#include "common/theme.h"
#include "common/uikit.h"
#include "common/easing.h"
#include "common/fonts.h"
#include "common/log.h"

#include <imgui.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cmath>

namespace clickgui {

static const char* TAG = "gui";
static bool g_open = false;
static double g_closed_at = -100.0;

struct Ripple { ImVec2 pos; double t0; };
static std::vector<Ripple> g_ripples;

bool is_open() { return g_open; }
float fx_amount() { return modules::manager().fx_amount; }
double time_since_close() { return client::now_sec() - g_closed_at; }

void toggle() {
    g_open = !g_open;
    if (!g_open) g_closed_at = client::now_sec();
    hooks::set_input_wants_mouse(g_open);
    clog::info(TAG, "gui %s", g_open ? "open" : "closed");
}

void init()    { g_ripples.clear(); }
void shutdown(){}

// ---------------------------------------------------------------------------
// rail
// ---------------------------------------------------------------------------
static const char* kCategories[] = { "Combat", "Movement", "Render", "Misc", "System" };
static int g_active = 1;

static void draw_rail(ImVec2 mn, ImVec2 mx, double now) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    uikit::chamfer_rect(dl, mn, mx, 12.f, theme::BG_GLASS,
                        theme::with_alpha(theme::ACCENT, 0.30f), 0.9f);
    uikit::dust(dl, mn, mx, (float)now, 18, theme::ACCENT, 99u);

    // brand block
    ImGui::PushFont(fonts::ui());
    const char* brand = "C R I M S O N";
    const ImVec2 bs = ImGui::CalcTextSize(brand);
    dl->AddText(ImVec2(mn.x + (mx.x - mn.x - bs.x) * 0.5f, mn.y + 18.f), theme::TEXT_HI, brand);
    ImGui::PopFont();
    dl->AddRectFilledMultiColor(ImVec2(mn.x + 16.f, mn.y + 46.f),
                                ImVec2(mx.x - 16.f, mn.y + 48.f),
                                theme::ACCENT, theme::ACCENT_HOT,
                                theme::with_alpha(theme::ACCENT_HOT, 0.f),
                                theme::with_alpha(theme::ACCENT, 0.f));

    // category items
    const float item_h = 40.f;
    const float y0 = mn.y + 74.f;
    for (int i = 0; i < 5; ++i) {
        const ImVec2 a(mn.x + 8.f, y0 + item_h * i);
        const ImVec2 b(mx.x - 8.f, a.y + item_h - 6.f);
        const bool active = i == g_active;
        const bool hov = ImGui::IsMouseHoveringRect(a, b);
        if (active) {
            dl->AddRectFilled(a, b, theme::with_alpha(theme::ACCENT, 0.12f), 6.f);
            // glowing active bar
            dl->AddRectFilled(ImVec2(a.x, a.y), ImVec2(a.x + 3.f, b.y),
                              theme::ACCENT, 1.5f);
            uikit::glow_rect(dl, ImVec2(a.x, a.y), ImVec2(a.x + 3.f, b.y), 1.5f,
                             theme::ACCENT, 0.6f, 0.f);
        } else if (hov) {
            dl->AddRectFilled(a, b, theme::with_alpha(theme::ACCENT, 0.05f), 6.f);
        }
        ImGui::SetCursorScreenPos(ImVec2(a.x + 12.f, a.y + (b.y - a.y) * 0.5f - ImGui::GetTextLineHeight() * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, active ? (ImVec4)ImColor(theme::TEXT_HI) : (ImVec4)ImColor(theme::TEXT_MID));
        ImGui::TextUnformatted(kCategories[i]);
        ImGui::PopStyleColor();
        if (hov && ImGui::IsMouseClicked(0)) g_active = i;
    }

    // version footer
    char v[48];
    snprintf(v, sizeof(v), "%s %s // %s", theme::NAME, theme::VERSION, theme::MC_VERSION);
    ImGui::PushFont(fonts::mono());
    const ImVec2 vs = ImGui::CalcTextSize(v);
    dl->AddText(ImVec2(mn.x + (mx.x - mn.x - vs.x) * 0.5f, mx.y - 26.f),
                theme::with_alpha(theme::TEXT_LOW, 0.8f), v);
    ImGui::PopFont();
}

// ---------------------------------------------------------------------------
// panels for the active category
// ---------------------------------------------------------------------------
static void module_panel(const char* category, ImVec2 mn, ImVec2 mx, double now) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    uikit::chamfer_rect(dl, mn, mx, 12.f, theme::BG_GLASS_2,
                        theme::with_alpha(theme::ACCENT, 0.35f), 0.9f);
    uikit::gradient_border(dl, ImVec2(mn.x, mn.y), ImVec2(mx.x, mn.y + 2.f),
                           fmodf((float)now * 0.3f, 1.f), theme::ACCENT, 1.6f, 0.3f);
    uikit::hatch(dl, mn, ImVec2(mx.x, mn.y + 34.f), 8.f,
                 theme::with_alpha(theme::ACCENT, 0.07f), (float)now * 6.f);

    // header
    ImGui::PushFont(fonts::ui());
    dl->AddText(ImVec2(mn.x + 16.f, mn.y + 10.f), theme::TEXT_HI, category);
    ImGui::PopFont();

    // module rows: toggle + keybind
    const float row_h = 30.f;
    float y = mn.y + 42.f;
    auto& mgr = modules::manager();
    const float W = mx.x - mn.x;

    for (auto& m : mgr.modules) {
        if (strcmp(m->category, category) != 0) continue;
        if (y + row_h > mx.y - 8.f) break;

        const ImVec2 a(mn.x + 10.f, y);
        const ImVec2 b(mx.x - 10.f, y + row_h - 4.f);
        const bool hov = ImGui::IsMouseHoveringRect(a, b);

        if (m->enabled) {
            dl->AddRectFilled(a, b, theme::with_alpha(theme::ACCENT, 0.10f), 6.f);
            dl->AddRect(a, b, theme::with_alpha(theme::ACCENT, 0.45f), 6.f, 0, 1.1f);
        } else if (hov) {
            dl->AddRectFilled(a, b, IM_COL32(255, 255, 255, 8), 6.f);
        }

        // state dot with pulse
        const float p = theme::pulse((float)now, 1.4f);
        dl->AddCircleFilled(ImVec2(a.x + 9.f, (a.y + b.y) * 0.5f),
                            m->enabled ? 3.f + 1.5f * p : 2.5f,
                            m->enabled ? theme::ACCENT : theme::TEXT_LOW);

        dl->AddText(ImVec2(a.x + 20.f, (a.y + b.y) * 0.5f - ImGui::GetTextLineHeight() * 0.5f),
                    m->enabled ? theme::TEXT_HI : theme::TEXT_MID, m->name);

        // settings hint (arrow) if module has any
        if (m->has_settings) {
            dl->AddText(ImVec2(b.x - 34.f, (a.y + b.y) * 0.5f - ImGui::GetTextLineHeight() * 0.5f),
                        theme::TEXT_LOW, ">");
        }

        if (hov && ImGui::IsMouseClicked(0)) {
            const ImVec2 mp = ImGui::GetIO().MousePos;
            g_ripples.push_back({ mp, now });
            if (mp.x < b.x - 44.f) {           // left zone = toggle
                m->set_enabled(!m->enabled, now);
            } else {                            // right zone = keybind capture mode
                m->binding = !m->binding;
            }
        }
        // keybind chip
        {
            char kb[32]{};
            if (m->binding) snprintf(kb, sizeof(kb), "...");
            else if (m->keybind) snprintf(kb, sizeof(kb), "[%c]", (char)m->keybind);
            else snprintf(kb, sizeof(kb), "[ ]");
            const ImVec2 ks = ImGui::CalcTextSize(kb);
            dl->AddText(ImVec2(b.x - ks.x - 10.f, (a.y + b.y) * 0.5f - ks.y * 0.5f),
                        m->binding ? theme::ACCENT : theme::TEXT_LOW, kb);
        }

        // per-module settings inline (expanded when binding slot clicked twice)
        if (m->binding && m->has_settings) {
            float sy = b.y + 2.f;
            m->draw_settings(sy, mx.x - 14.f);
            y = sy + 4.f;
            continue;
        }
        y += row_h;
    }

    // click ripples
    for (size_t i = 0; i < g_ripples.size();) {
        const float pr = (float)((now - g_ripples[i].t0) / 0.6);
        if (pr >= 1.f) { g_ripples.erase(g_ripples.begin() + i); continue; }
        uikit::ripple(dl, g_ripples[i].pos, pr, theme::ACCENT, 40.f);
        ++i;
    }
}

// ---------------------------------------------------------------------------
// system tab: fx sliders + mapping report
// ---------------------------------------------------------------------------
static void system_panel(ImVec2 mn, ImVec2 mx, double now) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    uikit::chamfer_rect(dl, mn, mx, 12.f, theme::BG_GLASS_2,
                        theme::with_alpha(theme::ACCENT, 0.35f), 0.9f);

    ImGui::PushFont(fonts::ui());
    dl->AddText(ImVec2(mn.x + 16.f, mn.y + 10.f), theme::TEXT_HI, "SYSTEM");
    ImGui::PopFont();

    float y = mn.y + 42.f;
    auto& mgr = modules::manager();
    const float x0 = mn.x + 16.f;
    const float w = mx.x - mn.x - 32.f;

    ImGui::SetCursorScreenPos(ImVec2(x0, y));
    if (widgets::slider("##fx", &mgr.fx_amount, 0.f, 1.f, "%.2f")) {}
    dl->AddText(ImVec2(x0, y - 16.f), theme::TEXT_MID, "post-fx intensity");
    y += 44.f;

    ImGui::SetCursorScreenPos(ImVec2(x0, y));
    static float hue = 0.985f;
    if (widgets::slider("##hue", &hue, 0.f, 1.f, "%.2f")) {
        gui_theme::set_accent_from_hue(hue);
    }
    dl->AddText(ImVec2(x0, y - 16.f), theme::TEXT_MID, "accent hue");
    y += 44.f;

    if (ImGui::SetCursorScreenPos(ImVec2(x0, y)), true) {
        const ImVec2 a(x0, y), b(x0 + 140.f, y + 26.f);
        const bool hov = ImGui::IsMouseHoveringRect(a, b);
        dl->AddRectFilled(a, b, theme::with_alpha(hov ? theme::BAD : theme::ACCENT, 0.14f), 6.f);
        dl->AddRect(a, b, theme::with_alpha(theme::BAD, 0.5f), 6.f, 0, 1.1f);
        dl->AddText(ImVec2(a.x + 10.f, a.y + 5.f), theme::TEXT_HI, "PANIC / UNINJECT");
        if (hov && ImGui::IsMouseHoveringRect(a, b) && ImGui::IsMouseClicked(0)) {
            request_uninject_async();
        }
        y += 40.f;
    }

    // mapping report (mono, scrollable list)
    ImGui::SetCursorScreenPos(ImVec2(x0, y));
    dl->AddText(ImVec2(x0, y), theme::TEXT_LOW, "mapping report");
    y += 18.f;
    ImGui::PushFont(fonts::mono());
    const float list_h = mx.y - y - 12.f;
    const ImVec2 lm(x0, y), lmx(mx.x - 14.f, y + list_h);
    static float scroll = 0.f;
    // simple scroll area
    ImGui::SetCursorScreenPos(lm);
    ImGui::BeginGroup();
    for (auto& line : mappings::report()) {
        const bool good = line.find(" OK ") != std::string::npos;
        dl->AddText(ImVec2(lm.x, lm.y + 0.f),
                    good ? theme::with_alpha(theme::OK, 0.8f) : theme::with_alpha(theme::BAD, 0.8f),
                    line.c_str());
        ImGui::Dummy(ImVec2(1, 14));
        ImGui::SetCursorScreenPos(ImVec2(lm.x, ImGui::GetCursorScreenPos().y));
    }
    ImGui::EndGroup();
    ImGui::PopFont();
}

// ---------------------------------------------------------------------------
// root
// ---------------------------------------------------------------------------
void draw(double now) {
    if (!g_open) return;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 W = ImGui::GetIO().DisplaySize;

    // pop-in animation
    const double age = now - (g_closed_at < 0 ? 0 : g_closed_at);
    (void)age;

    // window geometry
    const ImVec2 ws(760.f, 460.f);
    const ImVec2 wmn((W.x - ws.x) * 0.5f, (W.y - ws.y) * 0.5f);

    ImGui::SetNextWindowPos(wmn);
    ImGui::SetNextWindowSize(ws);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("##crimson_gui", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);

    // rail
    draw_rail(ImVec2(wmn.x, wmn.y), ImVec2(wmn.x + 190.f, wmn.y + ws.y), now);

    // panel area
    const ImVec2 pmn(wmn.x + 202.f, wmn.y);
    const ImVec2 pmx(wmn.x + ws.x, wmn.y + ws.y);
    if (g_active < 4) {
        const char* cat = kCategories[g_active];
        module_panel(cat, pmn, pmx, now);
    } else {
        system_panel(pmn, pmx, now);
    }

    // close hint
    ImGui::PushFont(fonts::mono());
    const char* hint = "INSERT close  //  END panic unload";
    const ImVec2 hs = ImGui::CalcTextSize(hint);
    dl->AddText(ImVec2((W.x - hs.x) * 0.5f, wmn.y + ws.y + 10.f),
                theme::with_alpha(theme::TEXT_LOW, 0.85f), hint);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();

    widgets::draw_toasts(now);
}

} // namespace clickgui

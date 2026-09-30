#include "client/gui/hud.h"
#include "client/gui/theme.h"
#include "client/gui/clickgui.h"
#include "client/gui/widgets.h"
#include "client/modules/module_manager.h"
#include "client/loader.h"
#include "common/theme.h"
#include "common/uikit.h"
#include "common/fonts.h"
#include <imgui.h>
#include <vector>
#include <string>
#include <cstdio>
#include <algorithm>

namespace hud {

static double g_boot = 0;
static float g_fps = 0;
static double g_fps_t0 = 0;
static int g_frames = 0;

void init()    { g_boot = client::now_sec(); }
void shutdown(){}

static void watermark(ImDrawList* dl, double now) {
    char text[96];
    snprintf(text, sizeof(text), "%s  //  %.0f FPS  //  %s",
             theme::NAME, g_fps, theme::MC_VERSION);
    ImGui::PushFont(fonts::mono());
    const ImVec2 ts = ImGui::CalcTextSize(text);
    const ImVec2 mn(14.f, 14.f), mx(14.f + ts.x + 34.f, 14.f + ts.y + 14.f);

    uikit::chamfer_rect(dl, mn, mx, 7.f, theme::with_alpha(theme::BG_GLASS_2, 0.85f),
                        theme::with_alpha(theme::ACCENT, 0.45f), 0.9f);
    uikit::gradient_border(dl, mn, ImVec2(mx.x, mn.y + 1.5f),
                           fmodf((float)now * 0.35f, 1.f), theme::ACCENT, 1.5f, 0.25f);
    dl->AddCircleFilled(ImVec2(mn.x + 11.f, (mn.y + mx.y) * 0.5f),
                        2.5f + 1.2f * theme::pulse((float)now, 1.8f), theme::ACCENT);
    dl->AddText(ImVec2(mn.x + 20.f, mn.y + 7.f), theme::TEXT_HI, text);
    ImGui::PopFont();
}

static void arraylist(ImDrawList* dl, double now) {
    auto& mgr = modules::manager();
    std::vector<const char*> names;
    for (auto& m : mgr.modules)
        if (m->enabled) names.push_back(m->name);
    if (names.empty()) return;
    std::sort(names.begin(), names.end(), [](const char* a, const char* b) {
        return strlen(a) > strlen(b);   // longest first, MC-style right alignment
    });

    ImGui::PushFont(fonts::mono());
    const ImVec2 W = ImGui::GetIO().DisplaySize;
    float y = 14.f;
    for (const char* n : names) {
        const ImVec2 ts = ImGui::CalcTextSize(n);
        const ImVec2 mn(W.x - ts.x - 30.f, y);
        const ImVec2 mx(W.x - 14.f, y + ts.y + 10.f);
        dl->AddRectFilled(mn, mx, theme::with_alpha(theme::BG_GLASS_2, 0.72f), 4.f);
        dl->AddRectFilled(ImVec2(mx.x - 2.f, mn.y), mx, theme::ACCENT, 1.f);
        dl->AddText(ImVec2(mn.x + 8.f, mn.y + 5.f), theme::TEXT_HI, n);
        y = mx.y + 4.f;
    }
    ImGui::PopFont();
}

void draw(double now) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    // fps counter
    ++g_frames;
    if (now - g_fps_t0 >= 0.5) {
        g_fps = (float)(g_frames / (now - g_fps_t0));
        g_fps_t0 = now;
        g_frames = 0;
    }

    if (!clickgui::is_open()) watermark(dl, now);
    arraylist(dl, now);
    widgets::draw_toasts(now);
}

} // namespace hud

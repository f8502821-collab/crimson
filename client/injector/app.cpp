#include "injector/app.h"
#include "injector/fxbg.h"
#include "injector/procfind.h"
#include "injector/inject.h"
#include "common/theme.h"
#include "common/uikit.h"
#include "common/easing.h"
#include "common/fonts.h"
#include "common/log.h"

#include <imgui.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace injector_app {

static const char* TAG = "app";

enum class Phase { Idle, Rescan, Injecting, Done, Failed };

struct Ripple { ImVec2 pos; double t0; };

struct State {
    Phase phase = Phase::Idle;
    std::vector<procfind::Proc> procs;
    int selected = -1;
    double last_scan = -10.0;
    double phase_t0 = 0.0;
    double now = 0.0;

    std::atomic<int>   step{ 0 };
    std::atomic<float> step_frac{ 0.f };
    std::atomic<bool>  worker_done{ false };
    std::atomic<bool>  worker_ok{ false };
    std::wstring       worker_err;
    std::thread        worker;

    ez::Spring scale;
    float ring = 0.f;
    std::vector<Ripple> ripples;

    char status[6][160];
    int  status_head = 0;
    int  status_count = 0;

    void push_status(const char* fmt, ...) {
        va_list a; va_start(a, fmt);
        char* slot = status[status_head];
        vsnprintf_s(slot, 160, _TRUNCATE, fmt, a);
        va_end(a);
        status_head = (status_head + 1) % 6;
        if (status_count < 6) ++status_count;
    }
};

static State g;

static std::wstring dll_path() {
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::filesystem::path p(buf);
    return (p.parent_path() / L"crimson_client.dll").wstring();
}

static void rescan_now() {
    g.procs = procfind::find_game_processes();
    if (g.selected >= (int)g.procs.size()) g.selected = g.procs.empty() ? -1 : 0;
    if (g.selected < 0 && !g.procs.empty()) g.selected = 0;
    g.last_scan = g.now;
}

static void worker_main(uint32_t pid, std::wstring dll) {
    inject::Options o;
    o.pid = pid;
    o.dll_path = dll;
    auto r = inject::run(o, [](int step, const char*, float frac, void* u) {
        auto* st = static_cast<State*>(u);
        st->step.store(step);
        st->step_frac.store(frac);
    }, &g);
    g.worker_err = r.error;
    g.worker_ok.store(r.ok);
    g.worker_done.store(true);
}

void init() {
    std::memset(g.status, 0, sizeof(g.status));
    g.scale.snap(1.f);
    g.now = ImGui::GetTime();
    rescan_now();
    g.push_status("crimson injector ready");
    if (g.procs.empty())
        g.push_status("waiting for a java process...");
    else
        g.push_status("target found: pid %u", g.procs[g.selected].pid);
    clog::info(TAG, "injector ui init");
}

void shutdown() {
    if (g.worker.joinable()) g.worker.join();
}

// ---------------------------------------------------------------------------
// pieces
// ---------------------------------------------------------------------------

static void draw_brand(ImVec2 pos, float w) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    char spaced[32]{};
    const char* name = theme::NAME;
    int o = 0;
    for (int i = 0; name[i] && o < 30; ++i) {
        spaced[o++] = name[i];
        if (name[i + 1]) spaced[o++] = ' ';
    }

    ImGui::PushFont(fonts::ui());
    const float big = 46.f;
    ImGui::SetWindowFontScale(big / ImGui::GetFontSize());
    const ImVec2 ts = ImGui::CalcTextSize(spaced);
    const float cx = pos.x + w * 0.5f - ts.x * 0.5f;

    // glow underlay
    for (int i = 4; i >= 1; --i) {
        dl->AddText(ImVec2(cx, pos.y), theme::with_alpha(theme::ACCENT, 0.035f * (5 - i)),
                    spaced);
        // offset copies for a soft bloom
    }
    dl->AddText(ImVec2(cx + 1.f, pos.y + 1.f), IM_COL32(0, 0, 0, 160), spaced);
    dl->AddText(ImVec2(cx, pos.y), theme::TEXT_HI, spaced);

    // underline gradient bar
    const float uy = pos.y + ts.y + 6.f;
    dl->AddRectFilledMultiColor(ImVec2(cx, uy), ImVec2(cx + ts.x, uy + 2.f),
                                theme::ACCENT, theme::ACCENT_HOT,
                                theme::with_alpha(theme::ACCENT_HOT, 0.0f),
                                theme::with_alpha(theme::ACCENT, 0.0f));
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopFont();

    ImGui::PushFont(fonts::mono());
    char sub[96];
    snprintf(sub, sizeof(sub), "FABRIC %s  //  SINGLEPLAYER UTILITY", theme::MC_VERSION);
    const ImVec2 ss = ImGui::CalcTextSize(sub);
    dl->AddText(ImVec2(pos.x + w * 0.5f - ss.x * 0.5f, uy + 8.f), theme::TEXT_LOW, sub);
    ImGui::PopFont();
}

static void status_dot(ImDrawList* dl, ImVec2 c, ImU32 col, double now) {
    const float p = theme::pulse((float)now, 1.6f);
    dl->AddCircleFilled(c, 5.f + 2.f * p, theme::with_alpha(col, 0.25f));
    dl->AddCircleFilled(c, 3.f, col);
}

static bool row(const procfind::Proc& p, bool selected, const ImVec2& mn, const ImVec2& mx) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    const bool hov = io.MousePos.x >= mn.x && io.MousePos.x <= mx.x &&
                     io.MousePos.y >= mn.y && io.MousePos.y <= mx.y;
    const bool clicked = hov && ImGui::IsMouseClicked(0);

    const float hov_a = uikit::hover_glow(mn, mx);
    if (selected)
        dl->AddRectFilled(mn, mx, theme::with_alpha(theme::ACCENT, 0.10f), 6.f);
    else if (hov_a > 0.01f)
        dl->AddRectFilled(mn, mx, theme::with_alpha(theme::ACCENT, 0.05f * hov_a), 6.f);

    const float cy = (mn.y + mx.y) * 0.5f;
    status_dot(dl, ImVec2(mn.x + 12.f, cy),
               p.has_window ? theme::OK : theme::WARN, g.now);

    char line[128];
    snprintf(line, sizeof(line), "%.*S", 40, p.has_window ? p.window_title.c_str() : L"(headless java)");
    ImGui::PushFont(fonts::mono());
    dl->AddText(ImVec2(mn.x + 26.f, cy - ImGui::GetFontSize() * 0.5f),
                selected ? theme::TEXT_HI : theme::TEXT_MID, line);
    char pid[32];
    snprintf(pid, sizeof(pid), "%u", p.pid);
    const ImVec2 ps = ImGui::CalcTextSize(pid);
    dl->AddText(ImVec2(mx.x - ps.x - 10.f, cy - ImGui::GetFontSize() * 0.5f),
                selected ? theme::ACCENT : theme::TEXT_LOW, pid);
    ImGui::PopFont();

    return clicked;
}

static void draw_target_card(ImVec2 mn, ImVec2 mx) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    uikit::chamfer_rect(dl, mn, mx, 10.f, theme::BG_GLASS, theme::with_alpha(theme::ACCENT, 0.35f), 0.8f);

    // header
    ImGui::PushFont(fonts::mono());
    dl->AddText(ImVec2(mn.x + 14.f, mn.y + 10.f), theme::TEXT_LOW, "TARGET");
    ImGui::PopFont();
    // spinning rescan arc, top-right
    {
        const ImVec2 c(mx.x - 22.f, mn.y + 20.f);
        const bool busy = g.phase == Phase::Rescan;
        const float ang = busy ? (float)(g.now - g.phase_t0) * 5.0f : 0.f;
        dl->PathClear();
        for (int i = 0; i <= 20; ++i) {
            const float a0 = ang + (float)i / 20.f * 4.7f;
            dl->PathLineTo(ImVec2(c.x + 7.f * cosf(a0), c.y + 7.f * sinf(a0)));
        }
        dl->PathStroke(theme::with_alpha(theme::ACCENT, busy ? 0.9f : 0.45f), 0, 2.f);
    }

    const ImVec2 list_mn(mn.x + 8.f, mn.y + 36.f);
    const ImVec2 list_mx(mx.x - 8.f, mx.y - 10.f);
    if (g.procs.empty()) {
        ImGui::PushFont(fonts::mono());
        dl->AddText(ImVec2(list_mn.x + 6.f, list_mn.y + 8.f), theme::TEXT_MID,
                    "no game detected - launch minecraft 1.21.11");
        ImGui::PopFont();
        return;
    }
    const int max_rows = 4;
    const int n = (int)g.procs.size() < max_rows ? (int)g.procs.size() : max_rows;
    const float rowh = (list_mx.y - list_mn.y) / (float)max_rows - 4.f;
    for (int i = 0; i < n; ++i) {
        ImVec2 a(list_mn.x, list_mn.y + (rowh + 4.f) * i);
        ImVec2 b(list_mx.x, a.y + rowh);
        if (row(g.procs[i], i == g.selected, a, b)) {
            g.selected = i;
            g.push_status("target set: pid %u", g.procs[i].pid);
        }
    }
}

static ImU32 phase_color() {
    switch (g.phase) {
    case Phase::Done:    return theme::OK;
    case Phase::Failed:  return theme::BAD;
    default:             return theme::ACCENT;
    }
}

static void draw_button(ImVec2 mn, ImVec2 mx) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();

    const bool has_target = !g.procs.empty() && g.selected >= 0;
    const bool busy = g.phase == Phase::Injecting;
    const ImU32 col = phase_color();

    // scale spring on hover/press
    const bool hov = io.MousePos.x >= mn.x && io.MousePos.x <= mx.x &&
                     io.MousePos.y >= mn.y && io.MousePos.y <= mx.y;
    const float target = hov && !busy ? 1.035f : 1.f;
    g.scale.step(target, 220.f, 22.f, (float)io.DeltaTime);
    const ImVec2 c((mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f);
    const ImVec2 half((mx.x - mn.x) * 0.5f * g.scale.value, (mx.y - mn.y) * 0.5f * g.scale.value);
    const ImVec2 a(c.x - half.x, c.y - half.y), b(c.x + half.x, c.y + half.y);

    // body: layered glass + accent wash
    uikit::glow_rect(dl, a, b, 0.f, col, busy ? 0.9f : (has_target ? 0.55f : 0.15f), busy ? 1.f : 0.6f);
    uikit::chamfer_rect(dl, a, b, 14.f,
                        busy ? theme::with_alpha(col, 0.16f)
                             : theme::with_alpha(theme::BG_GLASS_2, 0.9f),
                        theme::with_alpha(col, 0.55f), 0.9f);

    // marching border when idle+ready, orbit when busy
    if (!busy && has_target && g.phase != Phase::Done)
        uikit::gradient_border(dl, a, b, 0.f,
                               fmodf((float)g.now * 0.25f, 1.f), col, 1.6f);
    if (busy)
        uikit::gradient_border(dl, a, b, 0.f,
                               fmodf((float)g.now * 0.9f, 1.f), col, 2.2f, 0.5f);

    // hover shimmer
    if (hov && !busy && has_target)
        uikit::shimmer(dl, a, b, 0.f,
                       fmodf((float)g.now * 0.7f, 1.f), theme::TEXT_HI);

    // label
    const char* label = "INJECT";
    if (busy) label = "INJECTING";
    if (g.phase == Phase::Done) label = "INJECTED";
    if (g.phase == Phase::Failed) label = "FAILED";
    if (!has_target && !busy) label = "NO TARGET";

    ImGui::PushFont(fonts::ui());
    const float big = 26.f;
    ImGui::SetWindowFontScale(big / ImGui::GetFontSize());
    const ImVec2 ls = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2(c.x - ls.x * 0.5f, c.y - ls.y * 0.5f - 6.f),
                theme::TEXT_HI, label);
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopFont();

    // sublabel / progress
    char sub[96]{};
    if (busy) {
        static const char* steps[4] = { "allocating", "writing", "launching", "verifying" };
        const float frac = ((float)g.step.load() + g.step_frac.load()) / 4.f;
        g.ring = ez::damp(g.ring, frac * 0.999f + 0.001f, 6.f, (float)io.DeltaTime);
        snprintf(sub, sizeof(sub), "%s... %d%%", steps[g.step.load() % 4], (int)(g.ring * 100.f));
        // progress ring
        const float R = (b.x - a.x) * 0.5f + 14.f;
        dl->PathClear();
        dl->PathArcTo(c, R, -1.5708f, -1.5708f + 6.2831f * g.ring, 40);
        dl->PathStroke(theme::with_alpha(col, 0.9f), 0, 3.f);
        dl->AddCircle(c, R, theme::with_alpha(col, 0.18f), 0, 1.2f);
    } else if (g.phase == Phase::Done) {
        snprintf(sub, sizeof(sub), "press INSERT in game to open the menu");
    } else if (g.phase == Phase::Failed) {
        snprintf(sub, sizeof(sub), "%S", g.worker_err.c_str());
    } else if (has_target) {
        const float br = 0.5f + 0.5f * sinf((float)g.now * 2.4f);
        snprintf(sub, sizeof(sub), "ready  //  pid %u", g.procs[g.selected].pid);
        (void)br;
    } else {
        snprintf(sub, sizeof(sub), "waiting for minecraft");
    }
    ImGui::PushFont(fonts::mono());
    const ImVec2 ss = ImGui::CalcTextSize(sub);
    dl->AddText(ImVec2(c.x - ss.x * 0.5f, c.y + 8.f), theme::TEXT_MID, sub);
    ImGui::PopFont();

    // ripples
    for (size_t i = 0; i < g.ripples.size();) {
        const float p = (float)((g.now - g.ripples[i].t0) / 0.6);
        if (p >= 1.f) { g.ripples.erase(g.ripples.begin() + i); continue; }
        uikit::ripple(dl, g.ripples[i].pos, p, theme::TEXT_HI, 80.f);
        ++i;
    }

    // click handling
    if (hov && ImGui::IsMouseClicked(0) && has_target && !busy) {
        g.ripples.push_back({ io.MousePos, g.now });
        const std::wstring dll = dll_path();
        if (!std::filesystem::exists(dll)) {
            g.push_status("crimson_client.dll not found next to the injector");
            g.phase = Phase::Failed;
            g.phase_t0 = g.now;
            g.worker_err = L"DLL not found";
            return;
        }
        g.phase = Phase::Injecting;
        g.phase_t0 = g.now;
        g.ring = 0.f;
        g.worker_done.store(false);
        g.push_status("injecting into pid %u ...", g.procs[g.selected].pid);
        if (g.worker.joinable()) g.worker.join();
        const uint32_t pid = g.procs[g.selected].pid;
        g.worker = std::thread(worker_main, pid, dll);
    }
}

static void draw_status_log(ImVec2 mn, ImVec2 mx) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGui::PushFont(fonts::mono());
    float y = mx.y;
    for (int k = 0; k < g.status_count && y > mn.y; ++k) {
        const int idx = (g.status_head - 1 - k + 12) % 6;
        const float fade = k == 0 ? 1.f : 1.f - (float)k * 0.28f;
        const ImVec2 ts = ImGui::CalcTextSize(g.status[idx]);
        dl->AddText(ImVec2(mn.x, y - ts.y), theme::with_alpha(theme::TEXT_MID, fade),
                    g.status[idx]);
        y -= ts.y + 2.f;
    }
    ImGui::PopFont();
}

void draw() {
    g.now = ImGui::GetTime();
    ImGuiIO& io = ImGui::GetIO();

    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    fxbg::draw(bg, ImVec2(0, 0), io.DisplaySize, g.now);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::Begin("##crimson", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    const ImVec2 W = io.DisplaySize;
    const float pad = 42.f;
    const float cw = W.x - pad * 2.f;

    draw_brand(ImVec2(pad, 30.f), cw);

    draw_target_card(ImVec2(pad, 138.f), ImVec2(pad + cw, 258.f));

    draw_button(ImVec2(pad, 278.f), ImVec2(pad + cw, 372.f));

    draw_status_log(ImVec2(pad, 420.f), ImVec2(pad + cw, W.y - 28.f));

    // footer
    ImGui::PushFont(fonts::mono());
    const char* foot = "UNDETECTED BY DESIGN  //  SINGLE PLAYER ONLY  //  END = PANIC";
    const ImVec2 fs = ImGui::CalcTextSize(foot);
    ImGui::GetForegroundDrawList()->AddText(
        ImVec2((W.x - fs.x) * 0.5f, W.y - fs.y - 10.f), theme::with_alpha(theme::TEXT_LOW, 0.8f), foot);
    ImGui::PopFont();

    // phase transitions
    if (g.phase == Phase::Injecting && g.worker_done.load()) {
        if (g.worker.joinable()) g.worker.join();
        if (g.worker_ok.load()) {
            g.phase = Phase::Done;
            g.push_status("injected in %.2fs - press INSERT in game", g.now - g.phase_t0);
        } else {
            g.phase = Phase::Failed;
            g.push_status("injection failed - see log");
        }
        g.phase_t0 = g.now;
    }
    if ((g.phase == Phase::Done || g.phase == Phase::Failed) && g.now - g.phase_t0 > 8.0) {
        g.phase = Phase::Idle;
        rescan_now();
    }
    if (g.phase == Phase::Idle && g.now - g.last_scan > 2.0) rescan_now();

    ImGui::End();
    ImGui::PopStyleColor(2);
}

} // namespace injector_app

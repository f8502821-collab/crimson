// crimson_injector entry: layered window, D3D11 + ImGui, FX background.
#include "injector/app.h"
#include "common/log.h"
#include "common/fonts.h"

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <string>
#include <filesystem>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static const wchar_t* CLASS_NAME = L"CrimsonInjectorWnd";
static const char*    TAG = "main";

static HWND                  g_hwnd = nullptr;
static ID3D11Device*         g_dev = nullptr;
static ID3D11DeviceContext*  g_ctx = nullptr;
static IDXGISwapChain*       g_swap = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;

// forwarded from WndProc: exit when both a click lands outside and Esc pressed
static bool g_want_close = false;

static void create_rtv() {
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    ID3D11Texture2D* back = nullptr;
    g_swap->GetBuffer(0, IID_PPV_ARGS(&back));
    if (back) {
        g_dev->CreateRenderTargetView(back, nullptr, &g_rtv);
        back->Release();
    }
}

// pick a multibyte log path next to the exe
static std::string log_path() {
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::filesystem::path p(buf);
    return (p.parent_path() / "crimson_injector.log").string();
}

static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return 1;
    switch (m) {
    case WM_SIZE: return 0;
    case WM_SYSCOMMAND:
        if ((w & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        g_want_close = true;
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (w == VK_ESCAPE) g_want_close = true;
        break;
    }
    return DefWindowProcW(h, m, w, l);
}

static ATOM register_class(HINSTANCE inst) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wndproc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    return RegisterClassExW(&wc);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show) {
    if (clog::init(log_path().c_str()))
        clog::info(TAG, "injector starting");

    register_class(inst);

    const int W = 460, H = 520;
    RECT r{ 0, 0, W, H };
    AdjustWindowRectExForDpi(&r, WS_POPUP, FALSE, 0, USER_DEFAULT_SCREEN_DPI);

    g_hwnd = CreateWindowExW(
        WS_EX_APPWINDOW, CLASS_NAME, L"CRIMSON // injector",
        WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top,
        nullptr, nullptr, inst, nullptr);
    if (!g_hwnd) return 1;

    // rounded corners via DWM
    DWM_WINDOW_CORNER_PREFERENCE pref = DWMWCP_ROUND;
    DwmSetWindowAttribute(g_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(pref));

    ShowWindow(g_hwnd, show);
    UpdateWindow(g_hwnd);

    // --- D3D11 ---------------------------------------------------------------
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.Width = 0; sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = g_hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT flags = 0;
#ifdef CRIMSON_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL fl{};
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                             flags, nullptr, 0, D3D11_SDK_VERSION,
                                             &sd, &g_swap, &g_dev, &fl, &g_ctx))) {
        MessageBoxW(g_hwnd, L"D3D11 init failed", L"CRIMSON", MB_ICONERROR);
        return 1;
    }
    create_rtv();

    // --- ImGui ---------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    fonts::build();
    injector_app::init();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_dev, g_ctx);

    // frame loop --------------------------------------------------------------
    MSG msg{};
    ZeroMemory(&msg, sizeof(msg));
    while (!g_want_close) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) g_want_close = true;
        }
        if (g_want_close) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        injector_app::draw();

        ImGui::Render();
        const float clear[4] = { 0.f, 0.f, 0.f, 1.f };
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_ctx->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swap->Present(1, 0);
    }

    injector_app::shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (g_rtv) g_rtv->Release();
    if (g_swap) g_swap->Release();
    if (g_ctx) g_ctx->Release();
    if (g_dev) g_dev->Release();
    clog::shutdown();
    return 0;
}

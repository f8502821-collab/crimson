#include "common/fonts.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <filesystem>
#include <system_error>

namespace fonts {

// Directory of the module containing this code (works for both the injector
// exe and crimson_client.dll inside javaw.exe).
static std::filesystem::path own_module_dir() {
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&own_module_dir, &self);
    wchar_t buf[MAX_PATH]{};
    if (self && GetModuleFileNameW(self, buf, MAX_PATH))
        return std::filesystem::path(buf).parent_path();
    return {};
}

static std::string find_font(const wchar_t* name) {
    std::error_code ec;
    const auto own = own_module_dir();
    const std::filesystem::path candidates[] = {
        own / L"fonts" / name,
        own / name,
        std::filesystem::path(L"fonts") / name,
        std::filesystem::path(name),
    };
    for (const auto& p : candidates)
        if (std::filesystem::exists(p, ec)) return p.string();
    return {};
}

void build() {
    ImGuiIO& io = ImGui::GetIO();

    // UI (display) face at index fonts::UI
    const std::string gruppo = find_font(L"Gruppo-Regular.ttf");
    if (!gruppo.empty())
        io.Fonts->AddFontFromFileTTF(gruppo.c_str(), 21.f);
    else
        io.Fonts->AddFontDefault();

    // MONO face at index fonts::MONO
    const std::string jbmono = find_font(L"JetBrainsMono-Regular.ttf");
    if (!jbmono.empty())
        io.Fonts->AddFontFromFileTTF(jbmono.c_str(), 15.f);
    else
        io.Fonts->AddFontDefault();

    io.Fonts->Build();
}

} // namespace fonts

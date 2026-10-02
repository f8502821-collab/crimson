cmake_minimum_required(VERSION 3.24)

project(Crimson LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/deps.cmake)

# ---------------------------------------------------------------------------
# Target type helpers
# ---------------------------------------------------------------------------
macro(crimson_common_props tgt)
    target_compile_definitions(${tgt} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
        CRT_SECURE_NO_WARNINGS
        NOMSG
    )
    if(MSVC)
        target_compile_options(${tgt} PRIVATE
            /W3 /permissive- /Zc:preprocessor /EHsc /utf-8
            $<$<CONFIG:Release>:/O2 /Zi /DNDEBUG>
        )
        target_link_options(${tgt} PRIVATE
            $<$<CONFIG:Release>:/DEBUG:FULL /OPT:REF /OPT:ICF>
        )
    endif()
endmacro()

# ---------------------------------------------------------------------------
# common/ - shared static library (log, easing, theme, uikit)
# ---------------------------------------------------------------------------
add_library(crimson_common STATIC
    common/log.cpp
    common/easing.cpp
    common/uikit.cpp
    common/fonts.cpp
)
# theme.h is header-only; uikit.h/fonts.h include <imgui.h>, so common needs the
# imgui include path too (PUBLIC -> propagates to injector + client).
target_include_directories(crimson_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
crimson_common_props(crimson_common)

# ---------------------------------------------------------------------------
# injector/ - crimson_injector.exe (Win32 + D3D11 + ImGui)
# ---------------------------------------------------------------------------
add_executable(crimson_injector WIN32
    injector/main.cpp
    injector/app.cpp
    injector/fxbg.cpp
    injector/procfind.cpp
    injector/inject.cpp
)
target_include_directories(crimson_injector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}/include
)
target_sources(crimson_injector PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)
target_link_libraries(crimson_injector PRIVATE
    crimson_common
    d3d11.lib dxgi.lib d3dcompiler.lib
    user32.lib gdi32.lib shell32.lib ole32.lib
    dwmapi.lib
)
crimson_common_props(crimson_injector)

# ---------------------------------------------------------------------------
# client/ - crimson_client.dll (GL hooks + ImGui GL backend + MinHook + JNI)
# ---------------------------------------------------------------------------
add_library(crimson_client SHARED
    client/dllmain.cpp
    client/loader.cpp
    client/hooks/hooks.cpp
    client/jvm/jvm.cpp
    client/jvm/mappings.cpp
    client/jvm/mc.cpp
    client/render/renderer.cpp
    client/render/postfx.cpp
    client/gui/theme.cpp
    client/gui/widgets.cpp
    client/gui/clickgui.cpp
    client/gui/hud.cpp
    client/modules/module_manager.cpp
    client/modules/render/esp.cpp
    client/modules/render/tracers.cpp
    client/modules/render/nametags.cpp
    client/modules/render/chestesp.cpp
    client/modules/render/itemesp.cpp
    client/modules/render/glow.cpp
    client/modules/render/fullbright.cpp
    client/modules/movement/fly.cpp
    client/modules/movement/sprint.cpp
    client/modules/movement/speed.cpp
    client/modules/movement/nofall.cpp
    client/modules/movement/freecam.cpp
    client/modules/combat/aimassist.cpp
    client/modules/combat/autoclicker.cpp
    client/modules/combat/antiknockback.cpp
    client/modules/misc/timeoverride.cpp
    client/modules/misc/weatheroverride.cpp
    client/util/projection.cpp
    client/util/config.cpp
)
target_include_directories(crimson_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}/include
)
target_sources(crimson_client PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${minhook_SOURCE_DIR}/src/buffer.c
    ${minhook_SOURCE_DIR}/src/hook.c
    ${minhook_SOURCE_DIR}/src/trampoline.c
    ${minhook_SOURCE_DIR}/src/HDE/hde32.c
    ${minhook_SOURCE_DIR}/src/HDE/hde64.c
)
target_link_libraries(crimson_client PRIVATE
    crimson_common
    opengl32.lib
    user32.lib gdi32.lib shell32.lib psapi.lib
)
crimson_common_props(crimson_client)

# post.frag.h is generated from post.frag at build time
target_include_directories(crimson_client PRIVATE ${CMAKE_BINARY_DIR}/generated)
add_dependencies(crimson_client crimson_embed_shader)

# ---------------------------------------------------------------------------
# output layout: out/bin for both artifacts so injector finds its DLL easily
# ---------------------------------------------------------------------------
set_property(TARGET crimson_injector PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_property(TARGET crimson_client  PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_property(TARGET crimson_client  PROPERTY PREFIX "crimson_")
set_property(TARGET crimson_client  PROPERTY OUTPUT_NAME "client")

# ship the downloaded fonts next to both binaries (runtime fallback if absent)
add_custom_command(TARGET crimson_injector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRIMSON_FONT_DIR}
            $<TARGET_FILE_DIR:crimson_injector>/fonts
)
add_custom_command(TARGET crimson_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRIMSON_FONT_DIR}
            $<TARGET_FILE_DIR:crimson_client>/fonts
)

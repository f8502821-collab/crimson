

# crimsonhello! I would like to create a utility mod for 1.21.11 it will be used in single player only.
The client is made fully in c++, with the menu being imgui and injection handled by minhook. Any modules are implemented via JNI with a consistent logic that will not break.
DO NOT assume this is going to be updated. If i say a specific version like fabric 1.21.11 you do not need to care if it breaks on next update, we are not going to be updating.
@echo off
title CRIMSON builder
echo ============================================
echo   CRIMSON - one-click build
echo ============================================
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake was not found.
    echo Install Visual Studio 2022 with the "Desktop development with C++" workload,
    echo then run this file again.
    echo.
    pause
    exit /b 1
)

echo [1/2] Configuring...
cmake --preset msvc-x64-release
if errorlevel 1 (
    echo.
    echo [ERROR] Configure failed. Read the messages above.
    pause
    exit /b 1
)

echo.
echo [2/2] Building...
cmake --build --preset msvc-x64-release
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. Read the messages above.
    pause
    exit /b 1
)

echo.
echo ============================================
echo   DONE!
echo   Your files are in the  out\bin  folder:
echo     crimson_injector.exe   ^(run this^)
echo     crimson_client.dll     ^(keep next to it^)
echo ============================================
echo.
start "" explorer "%~dp0out\bin"
pause

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
    common/theme.cpp
    common/uikit.cpp
    common/fonts.cpp
)
target_include_directories(crimson_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
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

# ---------------------------------------------------------------------------
# output layout: out/bin for both artifacts so injector finds its DLL easily
# ---------------------------------------------------------------------------
set_property(TARGET crimson_injector PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_property(TARGET crimson_client  PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_property(TARGET crimson_client  PROPERTY PREFIX "crimson_" OUTPUT_NAME "client")

# ship the downloaded fonts next to both binaries (runtime fallback if absent)
add_custom_command(TARGET crimson_injector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRIMSON_FONT_DIR}
            $<TARGET_FILE_DIR:crimson_injector>/fonts
)
add_custom_command(TARGET crimson_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRIMSON_FONT_DIR}
            $<TARGET_FILE_DIR:crimson_client>/fonts
)

{
    "version": 3,
    "cmakeMinimumRequired": { "major": 3, "minor": 24, "patch": 0 },
    "configurePresets": [
        {
            "name": "msvc-x64-release",
            "displayName": "MSVC x64 Release (VS 2022)",
            "generator": "Visual Studio 17 2022",
            "architecture": { "value": "x64", "strategy": "set" },
            "binaryDir": "${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "msvc-x64-release",
            "configurePreset": "msvc-x64-release",
            "configuration": "Release"
        }
    ]
}

# CRIMSON

Utility client + injector for **Minecraft Fabric 1.21.11** (single player).
100% C++ / ImGui UI / MinHook / JNI. Not a Fabric mod — pure process injection.

## Build

Requirements: Windows 10/11, **Visual Studio 2022** (Desktop development with C++),
**CMake ≥ 3.24**. Internet needed once on the first configure (fetches ImGui + MinHook + fonts).

```bat
cmake --preset msvc-x64-release
cmake --build --preset msvc-x64-release
```

Output (both artifacts land together so the injector finds the DLL):

```
out\bin\crimson_injector.exe
out\bin\crimson_client.dll
```

## Use

1. Launch Minecraft **1.21.11** (single player world).
2. Run `crimson_injector.exe` — it auto-finds the game.
3. Press **INJECT**.
4. In game: **INSERT** toggles the menu, **END** panic-unloads.

Hotkeys: `B` ESP, `N` Tracers, `V` Fullbright (rebindable in the GUI).

## Modules (17)

| Category | Modules |
|---|---|
| Combat | AimAssist, AutoClicker, AntiKnockback |
| Movement | Fly, Sprint, Speed, NoFall, Freecam |
| Render | ESP, Tracers, Nametags, ChestESP, ItemESP, Glow, Fullbright |
| Misc | Time Override, Weather Override |

Plus core: ClickGUI, HUD watermark, arraylist, config save (`%APPDATA%\Crimson\config.ini`),
GLSL post-FX pass (chromatic aberration, vignette, grain, scanlines, crimson edge bleed),
accent-hue picker, FX intensity slider, mapping debug tab.

## Architecture

```
injector/   Win32 + D3D11 ImGui app. Auto process find, remote LoadLibrary inject.
client/     crimson_client.dll
  hooks/    MinHook: wglSwapBuffers (fallback glfwSwapBuffers) + WndProc subclass
  render/   ImGui GL3 backend, GLSL post-FX pass, per-frame orchestration
  gui/      ClickGUI + HUD + widgets (all custom-drawn, zero MC visuals)
  jvm/      jni_min.h (frozen ABI) + attach + mapping resolver + typed snapshots
  modules/  17 modules. Never touch JNI - consume snapshots only.
  util/     world->screen projection, config
common/     shared: logger, easing, theme, DrawList FX kit, fonts
```

**Safety model:** every JNI call is exception-guarded; mappings resolve through
candidate lists; a miss disables only the feature using it (check the System tab
mapping report). Module code never sees a JNIEnv.

## Fixing a mapping miss (1.21.11 intermediary names)

All names live in `client/jvm/mappings.cpp`. On a miss the log
(`%TEMP%\crimson_client.log`) prints e.g.:

```
[WARN][maps] ab.flying MISS field not found
```

Each entry accepts multiple candidates — add the correct `field_XXXX`/`method_XXXX`
to the list for that key (copy the line, add `"field_XXXX", "Z"` as another pair).
You can verify names from your Fabric run's `.jar`/mappings or
Linkie (https://linkie.shedaniel.dev) for 1.21.11.

High-confidence IDs used (stable across versions):
`class_310` MinecraftClient, `field_1724` player, `field_1687` world, `field_1774` options,
`class_1297` Entity (`method_19538/9/40` pos, `method_5829` bbox, `method_5705` isAlive,
`method_5729` setSprinting, `method_5744` getVelocity), `method_5829`+`field_1324..1329` Box,
`class_742` OtherClientPlayerEntity, `class_1309` LivingEntity (`method_5442` getHealth),
`class_1657` PlayerEntity, `class_2668` PlayerAbilities, `method_10260/1/2` BlockPos,
`field_1351..1353` Vec3d.

Candidate-list (may miss, each has fallbacks): gamma (SimpleOption `method_41723`/`method_42436`
+ legacy `field_1843`), fov (`method_42436` + legacy `field_2966`), yaw/pitch
(`method_36455..36458` + legacy `field_6036/6037`), abilities (`method_37303` + `field_7508`,
`field_6809/6812/6813`), fallDistance (`field_1323` D/F), hurtTime (`field_6009`),
world entity iter (`method_18112`), block entities (`method_46595`/`method_18233`),
time/rain setters (low confidence — Misc modules self-disable if absent),
setVelocity DDD (`method_5735`/`method_18803`), `doAttack` (`method_1583`).

## Notes

- Single player only. The client never sends packets beyond vanilla paths;
  server-authoritative actions (attack, movement) go through vanilla code.
- Fonts: Gruppo (display) + JetBrains Mono (numbers). If the font download failed
  at configure time the UI falls back to ImGui defaults.
- This project targets 1.21.11 and is never updated (per design).

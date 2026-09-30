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

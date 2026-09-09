# Codebase Index

## At a glance

- **Application:** One Windows console executable, `protocol.exe`.
- **Toolchain:** Visual Studio 2022, MSVC `v143`, Windows SDK, and C++20 for x64 builds.
- **Rendering:** Direct3D 11 with a bundled Dear ImGui implementation.
- **Runtime model:** Attach to the game process, cache Unreal Engine objects on a worker thread, and draw/configure features from an overlay render callback.
- **External packages:** Dear ImGui and nlohmann/json are committed to the repository; no package restore is required.
- **Tests/CI:** No automated test suite or CI configuration is present.

## Build entry points

| Path | Purpose |
| --- | --- |
| `protocol.sln` | Visual Studio solution with one project. |
| `protocol.vcxproj` | Source list and compiler settings. Release x64 adds the local `nlohmann` include directory. |
| `build-local.ps1` | Builds the current checkout with an installed Visual Studio 2022 C++ toolchain. |
| `build.bat` | Bootstrap script that downloads a fresh GitHub archive, builds it, copies the executable, and removes the downloaded tree. It is not the local incremental build path. |

Recommended local build:

```powershell
.\build-local.ps1 -Clean
```

The output is `x64\Release\protocol.exe`. Use `-Configuration Debug` for `x64\Debug\protocol.exe`.

## Runtime flow

1. `main.cpp` initializes item metadata and loads `config/config.json`.
2. `mem.hpp` locates the game window/process and resolves `LockdownProtocol-Win64-Shipping.exe`.
3. `overlay/dx11/moverlay.cpp` initializes the Medal overlay window, Direct3D 11, and ImGui.
4. `cache_useful()` in `main.cpp` refreshes world, player, item, and task pointers into the shared caches in `globals.h`.
5. `render_callback()` in `main.cpp` applies enabled features and renders ESP content from those caches.
6. `menu::draw()` and `radar::draw()` render the interactive configuration UI and radar.
7. The main thread continuously runs the overlay message loop.

## First-party source map

| Area | Paths | Responsibility |
| --- | --- | --- |
| Application orchestration | `main.cpp` | Entry point, process startup, cache thread, item metadata, feature logic, world-to-screen rendering, and overlay callback. |
| Menu | `menu.cpp`, `menu.h` | ImGui menu, feature controls, inventory editing, player list, config save/load actions, and class dump action. |
| Radar | `radar.cpp`, `radar.h` | Radar placement, transforms, markers, and drawing. |
| Configuration | `config.h`, `config/config.json`, `keybinds.hpp` | Defaults, hotkeys, colors, JSON persistence, and key-name mappings. |
| Shared runtime state | `globals.h`, `data_cache.h` | Resolved Unreal objects, synchronized object snapshots, cached item names, font state, and item lookup declarations. |
| Process memory | `mem.hpp`, `macros.hpp` | Window/process attachment, module discovery, remote reads/writes, and offset accessor macros. |
| Unreal SDK model | `game_structures.hpp`, `uobject.hpp`, `ufield.hpp`, `fstr_color_link.hpp` | Engine/game object layouts, offsets, inheritance model, and typed field accessors. |
| Unreal helpers | `game_function.hpp` | Object/function lookup, type checks, item-data lookup, ProcessEvent dispatch, teleport request helpers, and function dumps. |
| Math/utilities | `game_math.hpp`, `util.hpp`, `common.hpp` | Vectors, matrices, boxes, screen projection, name resolution, cursor/screen utilities, and shared declarations. |
| Game metadata | `ItemProperties.h`, `game_locations.h` | Item behavior values and named map locations. |

## Overlay and bundled code

| Path | Ownership | Responsibility |
| --- | --- | --- |
| `overlay/dx11/moverlay.cpp`, `overlay/dx11/moverlay.h` | Project-specific | Window discovery/creation, Direct3D device lifecycle, frame loop, input forwarding, and drawing helpers. |
| `overlay/overlay.h` | Project-specific | Overlay includes, Direct3D link directives, and shared overlay declarations. |
| `overlay/imgui/` | Vendored | Dear ImGui core plus Win32, DirectX 11, macOS, and Metal backend sources. Only the Win32/DirectX 11 sources are compiled here. |
| `nlohmann/` | Vendored | Header-only JSON parser used by `config.h`. |
| `iconv.h` | Vendored/reference | Libiconv compatibility header; it is not listed as a project source or included by the current first-party code. |

## Data and generated artifacts

| Path | Purpose |
| --- | --- |
| `fonts/MSYH.TTC` | Overlay font loaded at runtime. |
| `classnames.txt` | Captured Unreal class-name dump used for development/reference. |
| `function_search_output.txt` | Captured Unreal function search output. |
| `mec_functions_output.txt` | Captured MEC-related function dump. |
| `protocol.vcxproj.filters` | Visual Studio Solution Explorer grouping metadata. |

## Important state boundaries

- `config` owns user-controlled feature state and persists it to JSON.
- `mem` owns the target process handle, process ID, and module base.
- `globals` owns resolved game pointers and publishes synchronized cache snapshots consumed by rendering/menu code.
- `game_structures.hpp` is offset-sensitive: game updates can invalidate its constants and typed accessors without causing compile failures.
- `main.cpp` currently combines orchestration, cache population, feature writes, and ESP rendering; changes there should be validated against both cache and render paths.

## Performance model

- The overlay uses immediate presentation with a 144 FPS ceiling so menu input and animation are not tied to a 60 Hz swap chain.
- World/actor membership is refreshed four times per second; object positions and states are still read while rendering.
- Unreal arrays used by hot paths are copied with one remote-memory read instead of one read per element.
- Large actor sets are classified through the standard parallel execution pool before a single-threaded cache merge.
- Item and Unreal names are cached locally after their first successful resolution.
- Repeated gameplay memory writes run at 20 Hz, while weapon metadata writes run at 4 Hz.
- Cache publication uses snapshots so rendering never iterates a vector while the cache thread replaces it.

## Common change locations

- Add or change a menu option in `config.h`, `menu.cpp`, and both JSON save/load sections in `config.h`.
- Add a cached Unreal object type in `game_structures.hpp`, `globals.h`, and `cache_useful()` in `main.cpp`.
- Add an ESP renderer in `render_callback()` in `main.cpp`; add a radar marker in `radar.cpp` when needed.
- Change process or engine offsets in `game_structures.hpp` and related typed accessors.
- Change overlay lifecycle, input, or low-level drawing in `overlay/dx11/moverlay.cpp`.

## Runtime prerequisites

- Windows x64.
- The game must be running and expose the expected window/process names.
- Medal must be installed and running because the overlay searches for its window.
- `config/config.json` and `fonts/MSYH.TTC` must remain reachable from the process working directory.

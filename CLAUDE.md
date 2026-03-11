# CLAUDE.md — VoxelEngine Codebase Guide

This file provides guidance for AI assistants working in this repository.

---

## Project Overview

VoxelEngine is a Minecraft-style voxel game (~115k lines, C++17) featuring procedural terrain, multi-threaded chunk streaming, OpenGL 4.6 rendering, and survival mechanics. All assets are embedded into the binary at build time.

---

## Build System

**Requirements:** CMake 3.14+, C++17 compiler, OpenGL 4.6 capable GPU

```bash
# Standard build (Linux)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/src/VoxelEngine

# Optional: fetch GLFW automatically instead of using system package
cmake -B build -DGLFW_FETCH=ON
```

**Output binary:**
- Linux: `build/src/VoxelEngine`
- Windows: `build\src\Release\VoxelEngine.exe`

**Asset embedding:** Textures, shaders, and audio are converted to C++ headers via `cmake/embed_resources.cmake` (using `xxd`). The generated header is `build/embedded_assets.h`. Never reference asset files at runtime — always use the embedded data.

**Key CMake targets:**
- `VoxelEngine` — main game executable
- `glad` — OpenGL loader (static lib)
- `imgui` — debug UI (static lib)
- `zlibstatic` — compression (fetched at configure time)

---

## Directory Structure

```
VoxelEngine/
├── src/
│   ├── core/           # Entry point, renderer, world session, globals
│   ├── world/          # Chunks, terrain generation, biomes, caves, water, region I/O
│   ├── rendering/      # Meshing, camera, frustum, particles, item/tool models
│   │   └── opengl/     # Shader, VAO, VBO, EBO wrappers
│   ├── gameplay/       # Player, inventory, survival, raycasting
│   ├── audio/          # OpenAL-soft engine (pimpl), stb_vorbis decoder
│   ├── ui/             # HUD, debug menu (ImGui), main menu
│   ├── utils/          # Block type registry, job system, coordinate utilities
│   └── shaders/        # GLSL vertex/fragment shaders (14 files)
├── assets/
│   ├── textures/       # Block face PNGs, tool/weapon icons, destruction stages
│   └── sounds/         # .ogg audio files (footsteps, block, damage, ambient)
├── libs/
│   ├── glad/           # OpenGL function loader (in-tree)
│   ├── glm-1.0.2/      # Math library (header-only)
│   └── imgui/          # Dear ImGui (in-tree)
├── cmake/
│   └── embed_resources.cmake
└── CMakeLists.txt
```

---

## Architecture

### Initialization Flow

```
main()
  ├─ GLFW window + OpenGL context
  ├─ Renderer::init()          — shaders, texture array, VAOs
  ├─ WorldSession::init()      — ChunkManager, JobSystem, RegionManager
  ├─ AudioEngine::init()       — OpenAL device/context
  ├─ ParticleSystem::init()
  └─ Event loop (poll jobs, update, render, swap buffers)
```

### Key Subsystems

| System | Files | Role |
|---|---|---|
| **Renderer** | `core/Renderer.cpp` | All draw calls, shader uniforms, frame management |
| **ChunkManager** | `world/ChunkManager.h/cpp` | Chunk lifecycle: load, unload, mesh, track |
| **JobSystem** | `utils/JobSystem.h/cpp` | Thread pool for Generate/Mesh/Save tasks |
| **RegionManager** | `world/RegionManager.h/cpp` | Disk I/O, region file format, zlib compression |
| **TerrainGenerator** | `world/TerrainGenerator.h/cpp` | FBM noise terrain + biome decoration |
| **WaterSimulator** | `world/WaterSimulator.h/cpp` | Source/flow/falling liquid logic |
| **Player** | `gameplay/Player.h/cpp` | Position, physics, AABB, inventory, health/hunger |
| **AudioEngine** | `audio/AudioEngine.h/cpp` | OpenAL playback (pimpl pattern) |
| **ParticleSystem** | `rendering/ParticleSystem.h/cpp` | Instanced quad particles |

### Global State

`src/core/MainGlobals.h` declares `extern` pointers used across systems:

```cpp
extern Player*          g_player;
extern ChunkManager*    g_chunkManager;
extern ParticleSystem*  g_particleSystem;
extern AudioEngine*     g_audioEngine;
extern BlockTypes*      g_blockTypes;
```

These are set during initialization in `main.cpp` and accessed throughout the codebase. When adding new global systems, follow this same pattern.

### Threading Model

The `JobSystem` runs a thread pool where chunks are processed asynchronously:
- **Generate jobs** — terrain generation + decoration
- **Mesh jobs** — greedy mesh building (off-thread)
- **Save jobs** — region file writing

The main thread polls `pollCompletedMeshes()` each frame to upload finished VBOs. All OpenGL calls happen on the main thread only.

---

## Chunk System

- **Size:** 16×16×16 blocks per chunk
- **Coordinates:** World space uses `glm::ivec3`; chunk coords are `worldPos / CHUNK_SIZE`; local block coords are `worldPos % CHUNK_SIZE`
- **Storage:** `Chunk` struct holds a flat `uint8_t blocks[16*16*16]` array plus rendering VAO/VBO/EBO
- **Region files:** `saves/<worldname>/region/r.X.Z.bin` stores 32×32 chunks (4KB sectors, zlib-compressed)

Helper conversions are in `src/utils/CoordUtils.h`:
```cpp
glm::ivec3 worldToChunk(glm::ivec3 worldPos);
glm::ivec3 worldToLocal(glm::ivec3 worldPos);
```

---

## Block System

`src/utils/BlockTypes.h/cpp` defines a registry of 256 block types (ID 0 = air):

```cpp
struct BlockDef {
    std::string name;
    uint8_t     textureTop, textureSide, textureBottom;  // indices into texture array
    float       hardness;       // break time multiplier
    ToolType    preferredTool;  // PICKAXE, AXE, SHOVEL, NONE
    bool        transparent;
    bool        fluid;
    // ...
};
extern BlockTypes* g_blockTypes;  // g_blockTypes->getDef(id)
```

When adding new block types, add entries to the `BlockTypes` constructor and add corresponding textures to `assets/textures/`.

---

## Rendering Pipeline

### Shaders

All GLSL shaders live in `src/shaders/` and are embedded at build time:

| Shader pair | Purpose |
|---|---|
| `default.vert/frag` | Chunk geometry (lighting, biome tint) |
| `water.vert/frag` | Water with animated caustics |
| `selection.vert/frag` | Wireframe block selection highlight |
| `destroy.vert/frag` | Block destruction overlay (10 stages) |
| `particle.vert/frag` | Instanced quad particles |
| `item_model.vert/frag` | Inventory icon rendering |
| `tool_model.vert/frag` | First-person tool/weapon model |

### Vertex Format (chunk mesh)

Each vertex encodes: `position (vec3)`, `UV (vec2)`, `tileIndex (int)`, `skyLight (float)`, `faceShade (float)`, `biomeTint (vec3)`.

### Texture Array

Block faces are packed into a 2D texture array at startup. `BlockDef::textureTop/Side/Bottom` are layer indices into this array.

### Greedy Meshing

`src/rendering/Meshing.cpp` merges adjacent faces with matching block type and light into quads, reducing draw calls significantly. Meshing runs off the main thread via the job system.

---

## Game State

`src/core/GameState.h` defines:

```cpp
enum class GameState { MainMenu, WorldSelect, Playing, Paused, Settings };
```

The current state gates input handling and rendering in `main.cpp`. Use this enum when adding new screens or states.

---

## Audio

`src/audio/AudioEngine.h` uses the pimpl pattern — implementation details are hidden in `AudioEngine.cpp`. Public API:

```cpp
void playFootstep(BlockID surface);
void playBlockBreak(BlockID block);
void playBlockPlace(BlockID block);
void playDamage();
void playWaterSplash();
```

Audio files (`.ogg`) are embedded via the asset pipeline. Use `stb_vorbis` for decoding (already integrated in `src/audio/stb_vorbis_impl.c`).

---

## Player Physics

Constants in `src/gameplay/Player.h`:

```cpp
constexpr float PLAYER_WIDTH   = 0.6f;
constexpr float PLAYER_HEIGHT  = 1.8f;
constexpr float EYE_HEIGHT     = 1.62f;
constexpr float GRAVITY        = 28.0f;
constexpr float JUMP_VELOCITY  = 9.0f;
constexpr float TERMINAL_VEL   = 78.0f;
```

AABB collision is resolved per-axis (X, Y, Z separately). Modify physics constants here when tuning movement feel.

---

## Chat Commands

Parsed in `main.cpp` when the player presses Enter in chat:

| Command | Effect |
|---|---|
| `/noclip` | Toggle noclip / fly mode |
| `/time <0-24000>` | Set world time |
| `/gamemode <0|1>` | 0 = survival, 1 = creative |
| `/seed` | Print world seed |

Add new commands by extending the command parser block in `main.cpp`.

---

## Code Conventions

### Naming

| Element | Style | Example |
|---|---|---|
| Classes / Structs | PascalCase | `ChunkManager`, `BlockDef` |
| Functions / Methods | camelCase | `generateTerrain()`, `buildMesh()` |
| Constants | UPPER_SNAKE_CASE | `CHUNK_SIZE`, `MAX_SKY_LIGHT` |
| Global pointers | `g_` prefix | `g_player`, `g_chunkManager` |
| Member variables | no prefix | `position`, `velocity` |
| Local variables | camelCase | `chunkPos`, `blockId` |

### File Layout

- Headers (`.h`) declare classes and functions; implementations go in `.cpp`
- Use `#pragma once` at the top of every header
- Group includes: standard library → third-party (GLM, ImGui) → project headers
- Forward-declare types in headers where possible; include the definition in `.cpp`

### Style

- **Indentation:** 4 spaces (no tabs)
- **Braces:** Allman style (opening brace on its own line)
- **Line length:** No strict limit; aim for readability
- **Comments:** Only where logic is non-obvious; avoid restating what code does
- **Modern C++:** Use `std::unique_ptr`, `std::optional`, range-for, structured bindings where appropriate
- **GLM:** Always use `glm::vec3`, `glm::ivec3`, `glm::mat4` — never roll manual math

### Error Handling

This is a game, not a server. Fatal errors call `std::exit()` or throw and terminate. OpenGL errors are checked with `glGetError()` in debug builds. No exceptions are thrown in hot paths.

---

## Adding New Features — Checklist

### New Block Type
1. Add `BlockDef` entry in `BlockTypes` constructor (`src/utils/BlockTypes.cpp`)
2. Add texture PNG(s) to `assets/textures/` and register the file in `CMakeLists.txt` under the embedding list
3. Rebuild — `embed_resources.cmake` regenerates `embedded_assets.h`
4. Optionally update `TerrainGenerator` to place the block in terrain

### New Audio Event
1. Add `.ogg` file to `assets/sounds/`
2. Register it in `CMakeLists.txt` embedding list
3. Add a method to `AudioEngine` public API (`.h`) and implement it in `.cpp`
4. Call the method from the appropriate gameplay site

### New Shader
1. Add `name.vert` and `name.frag` to `src/shaders/`
2. Register both in `CMakeLists.txt` embedding list
3. Load via `ShaderClass` in `Renderer::init()`
4. Store the shader object in `Renderer` and use it in the appropriate render pass

### New World Feature (chunk-scale)
1. Implement in `src/world/` following the existing generator pattern
2. Call from `TerrainGenerator::generateTerrain()` after base terrain is placed
3. If it needs per-tick updates, add to the update loop in `main.cpp`

### New UI Screen
1. Add a value to the `GameState` enum
2. Add a render branch in `Renderer.cpp` and an input branch in `main.cpp`
3. Use ImGui for debug/dev screens; roll custom quads for production HUD elements

---

## Performance Notes

- **Frustum culling** is active — chunks outside the camera frustum are skipped entirely
- **Greedy meshing** merges faces; do not break this by mixing non-mergeable vertex attributes unnecessarily
- **Chunk radius** is configurable at runtime via the debug menu; keep default ≤ 12 for reasonable load times
- **Water simulation** is intentionally throttled — avoid triggering full-world water updates
- **OpenGL calls are main-thread only** — never call GL functions from job threads

---

## Common Pitfalls

- **Never call OpenGL from a job thread.** VBO uploads happen only after `pollCompletedMeshes()` on the main thread.
- **Block IDs are `uint8_t` (0–255).** ID 0 is always air. Don't exceed 255 block types.
- **Chunk coordinates vs. world coordinates.** Use `CoordUtils.h` helpers; mixing them causes silent bugs.
- **Asset embedding regenerates on CMake re-run.** If a new texture doesn't show up, re-run CMake, not just the compiler.
- **`g_blockTypes` is initialized before the game loop.** It is safe to read from worker threads (read-only after init).
- **`RegionManager` zlib streams must be flushed.** Always call `deflateEnd()` after writing a region sector.

---

## Test Framework

The project uses **Google Test** (fetched via CMake `FetchContent` at configure time). Tests live in `tests/` and cover pure-logic subsystems that have no OpenGL dependency.

### Running Tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc) --target voxel_tests
cd build && ctest --output-on-failure
# or run the binary directly:
./build/tests/voxel_tests
```

### Test Files

| File | What it covers |
|---|---|
| `tests/test_coord_utils.cpp` | `worldToChunk`, `worldToLocal`, `chunkToWorld` — including negative coords and round-trip invariants |
| `tests/test_block_types.cpp` | `initBlockTypes`, `isBlockSolid/Transparent/Liquid`, `getBlockHardness`, `getBlockPreferredTool`, `getToolSpeedMultiplier` |

### What Can Be Tested (and What Cannot)

**Testable without a GPU/display:**
- Coordinate math (`CoordUtils.h` — header-only, no GL calls)
- Block registry (`BlockTypes.cpp` — uses only TOOL_* integer constants from ToolModelGenerator.h, no GL calls)
- Any future pure-logic utility (inventory math, raycast geometry, etc.)

**Cannot be unit-tested without a context:**
- Anything that calls OpenGL (Renderer, ChunkManager mesh uploads, VAO/VBO wrappers)
- Audio (requires an OpenAL device)
- The job system's worker threads (depend on ChunkManager + RegionManager state)

For those, manual verification remains the approach:
- Launch with `--debug` to enable the ImGui debug menu
- Use `/noclip` + `/time` to explore generation artifacts
- Check chunk loading seams visually at region boundaries (multiples of 32 chunks)

### Adding New Tests

1. Create `tests/test_<module>.cpp`
2. Add it to `add_executable(voxel_tests ...)` in `tests/CMakeLists.txt`
3. If the module has `.cpp` files with no GL calls, add them to `voxel_testable` in `tests/CMakeLists.txt`
4. Use `TEST()` for stateless tests; use a `::testing::Test` fixture with `SetUpTestSuite()` when global state (like `initBlockTypes()`) must be initialised once

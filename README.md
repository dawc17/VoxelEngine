# VoxelEngine

Voxel chunk playground built with modern OpenGL 4.6. It renders greedy‑meshed 16×16×16 chunks using a 2D texture array sliced from `assets/textures/blocks.png` (32×32 tiles), overlays ImGui debug tools, and includes water simulation, particles, survival stats, and a lightweight in‑game chat/command system. GLAD, ImGui, and GLM are vendored under `libs/`.

<img width="2559" height="1387" alt="image" src="https://github.com/user-attachments/assets/1c89a0ac-84b4-4e1b-8ad4-b8de7c35f59a" />
<img width="2559" height="1365" alt="image" src="https://github.com/user-attachments/assets/e32e9d99-66db-4261-8e2c-b1228f1df731" />
<img width="2559" height="1386" alt="image" src="https://github.com/user-attachments/assets/7da748df-7503-4248-abf5-09700a6acd37" />

## Note about AI Usage
This README was generated using AI. AI was also used for debugging when the developer was in absolute despair and could not for his life figure out what the hell an euler angle was or how exactly physics work. I hope you understand. Other than that, the code is 80% human effort.

## Features
- **Chunk Streaming**: Streams a configurable radius of chunks around the player with async load/save through a multi‑threaded job system.
- **Procedural Terrain**: FBM‑style terrain with stone/dirt/grass/sand strata, sea level water fill, and tree decorator.
- **Cave Generation**: Minecraft‑inspired caves (cheese + spaghetti) carved underground.
- **Greedy Meshing**: Merges adjacent faces with the same texture to reduce draw calls.
- **Texture Array Atlas**: `blocks.png` split into 1024 tiles (32×32) with mipmaps + anisotropic filtering.
- **Water System**: Source + flowing levels with edge search, falling water, and optional caustics.
- **Particles**: Instanced quads for block break particles.
- **Survival System**: Health + hunger bars, fall damage, drowning, regen, and respawn.
- **Player Controller**: AABB collision, gravity, jumping, noclip.
- **Block Interaction**: Raycast selection with wireframe highlight, LMB break / RMB place.
- **Day/Night Cycle**: Dynamic sky, fog, and ambient lighting.
- **Debug UI**: ImGui panels for stats, settings, and “visual chaos” modes.
- **Chat + Commands**: In‑game chat with a few built‑in commands.

## Prerequisites
- CMake 3.14+
- C++17 compiler and OpenGL 4.6 capable GPU/drivers
- GLFW development package (or enable `-DGLFW_FETCH=ON`; defaults to ON on Windows, OFF elsewhere)
- Linux packages (Debian/Ubuntu):  
  `sudo apt install/yay -S build-essential cmake libglfw3-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev`

## Build
```bash
cmake -B build -S . [-DGLFW_FETCH=ON]
cmake --build build       # produces build/src/VoxelEngine
```
Pass `-DGLFW_FETCH=ON` if you want CMake to fetch/build GLFW (useful on Windows or when a system package is unavailable).

### Windows (VS2022 Build Tools)
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DGLFW_FETCH=ON .
cmake --build build --config Release
build\src\Release\VoxelEngine.exe
```
If you already have GLFW installed, set `-DGLFW_FETCH=OFF` and ensure CMake can find `glfw3`.

## Run
```bash
./build/src/VoxelEngine
```
- Release builds on Windows land at `build\src\Release\VoxelEngine.exe`; Debug builds live in `build\src\Debug\VoxelEngine.exe`.
- Shaders and assets are copied to the build output directory at build time.

## Distribution
The `build/src/Release/` folder is self‑contained and ready to distribute:
```text
Release/
├── VoxelEngine.exe
├── shaders/
│   ├── default.vert
│   ├── default.frag
│   ├── selection.vert
│   ├── selection.frag
│   ├── water.vert
│   └── water.frag
└── assets/
    └── textures/
        ├── blocks.png
        └── hud_blocks/
            └── *.png
```
Zip the `Release/` folder and share it. Saves are created in a `saves/` folder next to the executable.

## Controls
| Key | Action |
|-----|--------|
| `W/A/S/D` | Move player |
| `Mouse` | Look around (cursor locked by default) |
| `Space` | Jump (or move up in noclip) |
| `Left Shift` | Move down (noclip only) |
| `Mouse Wheel` | Cycle placeable blocks |
| `LMB` | Break block |
| `RMB` | Place block |
| `U` | Toggle cursor lock |
| `Tab` | Toggle debug menu |
| `T` | Open chat input |
| `R` | Respawn when dead |
| `Esc` | Close chat (if open) or exit |

## Chat Commands
- `/noclip` — toggle noclip
- `/time set <0..1>` or `/time day|noon|sunset|night|sunrise`
- `/gamemode survival|creative|0|1`
- `/seed` — print terrain seed

## Debug Menu
The ImGui debug window provides:
- FPS, player position/velocity/orientation
- Chunk coordinates and selection info
- Wireframe mode toggle
- Noclip mode toggle
- Async loading toggle
- Move speed slider
- Max FPS limiter
- Day/night controls (auto time, time of day, day length, fog density)
- Water controls (simulation tick rate, caustics)
- Fun modes (Drunk / Disco / Earthquake)
- Block texture randomizer/reset

## Survival HUD
- Health + hunger bars at the bottom left
- Drowning damage when fully submerged
- Fall damage and hunger drain
- “You died – press R” respawn hint

## Project Layout
```text
src/
├── main.cpp                  # Main loop, rendering, input, ImGui UI
├── MainGlobals.*             # Global state, input, chat, HUD icons
├── Chunk.*, ChunkManager.*   # Chunk storage and management
├── Meshing.*                 # Greedy meshing
├── BlockTypes.*              # Block definitions and atlas mapping
├── TerrainGenerator.*        # Procedural terrain + trees
├── CaveGenerator.*           # Cheese + spaghetti caves
├── WaterSimulator.*          # Water simulation
├── ParticleSystem.*          # Block break particles
├── SurvivalSystem.*          # Health/hunger logic + HUD
├── Player.*, Camera.*        # Player physics and camera
├── Raycast.*                 # Block raycasting
├── JobSystem.*               # Multi-threaded async loading
├── RegionManager.*           # World save/load with zlib compression
├── ShaderClass.*, VBO.*, VAO.*, EBO.*  # OpenGL helpers
└── *.vert, *.frag            # GLSL shaders
```

## Troubleshooting
- **Texture/shader not found**: Run from the repo root or `build/` so relative paths resolve; ensure `assets/textures/blocks.png` exists.
- **GLFW link errors**: Install the GLFW dev package or enable `-DGLFW_FETCH=ON`.
- **Caves breaking through surface**: Adjust `surfaceMargin` in `CaveGenerator.h`.
- **Performance issues**: Reduce render distance or enable async loading in the debug menu.
# Engine

Voxel chunk playground built with modern OpenGL 4.6. Renders greedy-meshed 16x16x16 chunks using a texture atlas and ImGui for debug controls. GLAD, ImGui, and GLM are vendored under `libs/`.

## Features
- Streams chunks around the camera and uploads meshes on demand (greedy meshing + element/index buffers).
- Texture atlas at `assets/textures/atlas.png` is converted to a 2D texture array with anisotropic filtering.
- FPS-style camera (`WASD` + mouse look) with runtime camera speed slider and cursor lock toggle.
- ImGui debug window shows FPS, camera position/orientation, current chunk, and a wireframe toggle.
- Shaders live in `src/default.vert` and `src/default.frag` with `SHADER_DIR` baked into the binary for easy running from the repo or `build/`.

## Prerequisites
- CMake 3.14+
- C++17 compiler and OpenGL 4.6 capable GPU/drivers
- GLFW development package (or enable `-DGLFW_FETCH=ON` to fetch/build it; defaults to ON on Windows, OFF elsewhere)
- Linux packages (Debian/Ubuntu): `sudo apt install build-essential cmake libglfw3-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev`

## Build
```bash
cmake -B build -S .
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
- Running from the repo root or `build/` works for both shaders and the atlas thanks to `SHADER_DIR` and the relative texture lookup in `MAIN.cpp`.

## Controls
- `W/A/S/D`: fly the camera
- Mouse: look around (cursor locked by default)
- `U`: toggle cursor lock/unlock (re-centers on lock)
- `Esc`: close the window
- ImGui debug window: wireframe toggle, camera speed slider, FPS/camera/chunk readouts

## Project Layout
- `src/MAIN.cpp`: main loop, chunk streaming, texture array setup, ImGui wiring
- `src/Chunk*`, `Meshing.*`, `BlockTypes.*`: chunk storage, greedy meshing, and block/atlas mapping
- `src/ShaderClass.*`, `VBO.*`, `VAO.*`, `EBO.*`: OpenGL helpers
- `assets/textures/`: atlas and reference textures

## Troubleshooting
- Shader/atlas not found: run from the repo root or `build/` so relative paths resolve; ensure `assets/textures/atlas.png` and `src/default.vert/.frag` exist.
- GLFW link errors: install the GLFW dev package (or enable `-DGLFW_FETCH=ON` to build it locally).

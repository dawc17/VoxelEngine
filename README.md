# VoxelEngine

Small OpenGL playground that draws a simple indexed strip with ImGui controls. Built with CMake, GLFW, GLAD, and ImGui (vendored under `libs/`).

## Prerequisites
- CMake 3.14+
- A C++17 compiler
- OpenGL 4.6 capable GPU/drivers
- GLFW development package installed on the system (or let CMake fetch/build it on Windows)

## Build
```bash
mkdir -p build
cd build
cmake ..            # configure
cmake --build .     # compile (produces build/src/VoxelEngine)
```

## Run
```bash
cd build
./src/VoxelEngine
```
- Shaders are loaded from `src/default.vert` and `src/default.frag`. The binary now embeds the source shader directory (`SHADER_DIR`), so running from either the repo root or `build/` works. If you add/move shader files, update `src/CMakeLists.txt` accordingly.

### Windows (VS2022 Build Tools)
- Install: Visual Studio 2022 Build Tools (with C++ workload), CMake, and Git.
- Configure & build (will fetch/build GLFW automatically):
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DGLFW_FETCH=ON .
cmake --build build --config Release
```
- Run: `build\src\Release\VoxelEngine.exe`
- If you already have GLFW installed, set `-DGLFW_FETCH=OFF` and ensure `glfw3` is discoverable by CMake.

## Controls
- ESC: close the window
- ImGui window: toggle wireframe mode

## Development Notes
- When adding shaders, place them in `src/` or adjust `SHADER_DIR` in `src/CMakeLists.txt`.
- Libraries are vendored in `libs/` except GLFW, which can be fetched automatically on Windows via `GLFW_FETCH`.
- Core rendering setup lives in `src/MAIN.cpp`; shader utilities are in `src/ShaderClass.*`.

## Troubleshooting
- **Shader file errors**: confirm `src/default.vert` and `src/default.frag` exist and that `SHADER_DIR` still points to `src/`.
- **Linker errors for GLFW**: ensure the GLFW development package is installed (e.g., `sudo apt install libglfw3-dev` on Debian/Ubuntu) or configure with `-DGLFW_FETCH=ON` to build GLFW locally on Windows.

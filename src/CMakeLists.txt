set(SOURCES
  MAIN.cpp
  VAO.cpp
  VBO.cpp
  EBO.cpp
  ShaderClass.cpp
)

add_executable(VoxelEngine ${SOURCES})

# === GLAD ===
add_library(glad ../libs/glad/src/glad.c)
target_include_directories(glad PUBLIC ../libs/glad/include)

# === Include directories ===
target_include_directories(VoxelEngine PUBLIC
    ../libs/glad/include
    ../libs/glm
)

# === GLFW & OpenGL ===
find_package(glfw3 REQUIRED)
if(WIN32)
    target_link_libraries(VoxelEngine glad glfw opengl32)
else()
    target_link_libraries(VoxelEngine glad glfw GL)
endif()


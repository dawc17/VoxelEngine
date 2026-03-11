include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Static library containing pure-logic source files that have no GL calls.
# BlockTypes.cpp pulls in ToolModelGenerator.h (for TOOL_* constants) which
# includes glad/glad.h — that is fine for compilation because we only need
# GLuint typedef definitions; we never invoke any GL function from these files.
add_library(voxel_testable STATIC
    ${CMAKE_SOURCE_DIR}/src/utils/BlockTypes.cpp
)
target_include_directories(voxel_testable PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/src/rendering
    ${CMAKE_SOURCE_DIR}/libs/glad/include
)
target_link_libraries(voxel_testable PUBLIC glm::glm)

add_executable(voxel_tests
    test_coord_utils.cpp
    test_block_types.cpp
)
target_include_directories(voxel_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/src/rendering
    ${CMAKE_SOURCE_DIR}/libs/glad/include
)
target_link_libraries(voxel_tests PRIVATE
    voxel_testable
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(voxel_tests)

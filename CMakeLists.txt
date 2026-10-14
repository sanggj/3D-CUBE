cmake_minimum_required(VERSION 3.16)

project(CubeChaos)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CubeChaos
    src/main.cpp
    src/shader.cpp
    src/instances.cpp
    src/msaa.cpp
    src/input.cpp
    src/ui.cpp

    lib/glad/src/glad.c

    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(CubeChaos PRIVATE
    lib/glad/include
    lib/imgui
    lib/imgui/backends
)

find_package(OpenGL REQUIRED)

add_subdirectory(lib/glfw)

target_link_libraries(CubeChaos PRIVATE
    glfw
    OpenGL::GL
)

if (WIN32)
    target_link_libraries(CubeChaos PRIVATE gdi32)
endif()

enable_testing()

add_executable(CubeTests test/cube_tests.cpp)

add_test(NAME CubeGeometryAndMath COMMAND CubeTests)

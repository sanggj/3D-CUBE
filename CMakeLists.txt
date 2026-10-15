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
    Threads::Threads
)

if (WIN32)
    target_link_libraries(CubeChaos PRIVATE gdi32)
endif()

if (MSVC)
    target_compile_options(CubeChaos PRIVATE /W4)
    set_source_files_properties(
        lib/glad/src/glad.c
        lib/imgui/imgui.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/imgui_demo.cpp
        lib/imgui/backends/imgui_impl_glfw.cpp
        lib/imgui/backends/imgui_impl_opengl3.cpp
        PROPERTIES COMPILE_OPTIONS "/w"
    )
else()
    target_compile_options(CubeChaos PRIVATE -Wall -Wextra)
    set_source_files_properties(
        lib/glad/src/glad.c
        lib/imgui/imgui.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/imgui_demo.cpp
        lib/imgui/backends/imgui_impl_glfw.cpp
        lib/imgui/backends/imgui_impl_opengl3.cpp
        PROPERTIES COMPILE_OPTIONS "-w"
    )
endif()

find_package(Threads REQUIRED)

enable_testing()

add_executable(CubeTests
    test/cube_tests.cpp
    src/instances.cpp
)

target_link_libraries(CubeTests PRIVATE Threads::Threads)

if (MSVC)
    target_compile_options(CubeTests PRIVATE /W4)
else()
    target_compile_options(CubeTests PRIVATE -Wall -Wextra)
endif()

add_test(NAME CubeGeometryAndMath COMMAND CubeTests)

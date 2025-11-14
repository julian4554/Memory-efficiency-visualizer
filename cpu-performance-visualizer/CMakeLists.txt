cmake_minimum_required(VERSION 3.22)
project(cpu_performance_visualizer C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Statisch linken
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -Wl,-subsystem,console")

# --- SDL2 ---
find_package(SDL2 CONFIG REQUIRED)

# --- ImGui + ImPlot ---
include(FetchContent)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.0
)

FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
)

FetchContent_MakeAvailable(imgui implot)

# --- Core Library (measure + runner) ---
add_library(core STATIC
        src/core/measure.c
        src/core/measure.h
        src/core/measure_runner.cpp
        src/core/measure_runner.h
)

target_include_directories(core PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/utils
)

target_link_libraries(core PUBLIC
        m
)

# --- Executable (GUI) ---
add_executable(cpu_performance_visualizer
        src/app/main.cpp
        src/app/gui.cpp
        src/app/gui.h

        # ImGui backend
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp

        # ImPlot
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(cpu_performance_visualizer PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${implot_SOURCE_DIR}
)

target_link_libraries(cpu_performance_visualizer
        PRIVATE core
        SDL2::SDL2
        SDL2::SDL2main
        opengl32
        m
)

# --- Tests ---
add_subdirectory(tests)

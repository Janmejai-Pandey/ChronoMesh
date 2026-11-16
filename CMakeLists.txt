cmake_minimum_required(VERSION 3.16)
project(ChronoMesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Reconfigure after editing this list.
set(CORE_SOURCES
    src/graph/Graph.cpp          # Person 1 (currently a temporary stub)
    src/scheduler/Scheduler.cpp  # Person 2
)

set(UI_SOURCES
    src/execution/Execution.cpp
    src/execution/Analytics.cpp
    src/execution/Benchmark.cpp
    ui/src/Visualization.cpp
)

# Original team targets: keep these source lists unchanged.
add_executable(ChronoMesh src/main.cpp ${CORE_SOURCES})
add_executable(test_scheduler
    tests/test_scheduler.cpp
    ${CORE_SOURCES}
)
target_include_directories(ChronoMesh PRIVATE include)
target_include_directories(test_scheduler PRIVATE include)

enable_testing()
add_test(NAME test_scheduler COMMAND test_scheduler)

# UI target is separate so it does not replace the team's ChronoMesh entry point.
add_executable(ChronoMeshUI
    ui/src/main.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
)
target_include_directories(ChronoMeshUI PRIVATE include)

add_executable(ChronoMeshUITests
    tests/ChronoMeshTests.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
)
target_include_directories(ChronoMeshUITests PRIVATE include)
add_test(NAME ChronoMeshUITests COMMAND ChronoMeshUITests)

# The desktop interface is optional so the original team targets stay usable
# without downloading graphics libraries. Enable it with
# -DCHRONOMESH_BUILD_DESKTOP_UI=ON.
option(CHRONOMESH_BUILD_DESKTOP_UI "Build the GLFW and Dear ImGui desktop UI" OFF)

if(CHRONOMESH_BUILD_DESKTOP_UI)
    # GLFW is an external C library; the application and project modules remain C++.
    enable_language(C)

    set(CHRONOMESH_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
    set(CHRONOMESH_GLFW_SOURCE_DIR "${CHRONOMESH_THIRD_PARTY_DIR}/glfw")
    set(CHRONOMESH_IMGUI_SOURCE_DIR "${CHRONOMESH_THIRD_PARTY_DIR}/imgui")

    if(NOT EXISTS "${CHRONOMESH_GLFW_SOURCE_DIR}/CMakeLists.txt" OR
       NOT EXISTS "${CHRONOMESH_IMGUI_SOURCE_DIR}/imgui.cpp")
        message(FATAL_ERROR
            "The local GLFW or Dear ImGui source files are missing from third_party/. "
            "Restore third_party/glfw and third_party/imgui before building the desktop UI.")
    endif()

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CHRONOMESH_GLFW_SOURCE_DIR}"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party-glfw" EXCLUDE_FROM_ALL)

    # Dear ImGui is a small source-only library, so compile its core and the
    # official GLFW/OpenGL3 backends directly into a local static library.
    add_library(ChronoMeshDearImGui STATIC
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/imgui.cpp
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/imgui_draw.cpp
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/imgui_tables.cpp
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/imgui_widgets.cpp
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(ChronoMeshDearImGui PUBLIC
        ${CHRONOMESH_IMGUI_SOURCE_DIR}
        ${CHRONOMESH_IMGUI_SOURCE_DIR}/backends
    )
    target_link_libraries(ChronoMeshDearImGui PUBLIC glfw)

    find_package(OpenGL REQUIRED)
    add_executable(ChronoMeshDesktop
        ui/src/desktop_main.cpp
        ${CORE_SOURCES}
        ${UI_SOURCES}
    )
    target_include_directories(ChronoMeshDesktop PRIVATE include)
    target_link_libraries(ChronoMeshDesktop PRIVATE ChronoMeshDearImGui OpenGL::GL)
endif()

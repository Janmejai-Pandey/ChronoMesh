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

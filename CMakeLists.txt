cmake_minimum_required(VERSION 3.16)
project(ChronoMesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reconfigure after editing this list.
set(CORE_SOURCES
    src/graph/Graph.cpp          # Person 1 (currently a temporary stub)
    src/scheduler/Scheduler.cpp  # Person 2
)

add_executable(ChronoMesh src/main.cpp ${CORE_SOURCES})
target_include_directories(ChronoMesh PRIVATE include)

add_executable(test_scheduler tests/test_scheduler.cpp ${CORE_SOURCES})
target_include_directories(test_scheduler PRIVATE include)
target_sources(test_scheduler PRIVATE
    src/scheduler/EstimateCompletion.cpp
    src/execution/Execution.cpp
)

# The UI is a separate target so the existing team targets above stay intact.
set(CHRONOMESH_UI_CORE_SOURCES
    ${CORE_SOURCES}
    src/scheduler/EstimateCompletion.cpp
    src/execution/Execution.cpp
    src/execution/Analytics.cpp
    src/execution/Benchmark.cpp
    ui/src/Visualization.cpp
)

add_executable(ChronoMeshUI
    ui/src/main.cpp
    ${CHRONOMESH_UI_CORE_SOURCES}
)
target_include_directories(ChronoMeshUI PRIVATE include)

enable_testing()
add_executable(ChronoMeshUITests
    tests/ChronoMeshTests.cpp
    ${CHRONOMESH_UI_CORE_SOURCES}
)
target_include_directories(ChronoMeshUITests PRIVATE include)
add_test(NAME ChronoMeshUITests COMMAND ChronoMeshUITests)

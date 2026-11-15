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
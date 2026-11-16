# ChronoMesh User Interfaces

The project has a small terminal interface and a graphical desktop interface. The desktop interface is written in C++ with GLFW, Dear ImGui, and OpenGL. It shows the task dependency graph, worker timelines, execution metrics, and scheduler benchmarks.

## Build the graphical interface on Windows with MinGW

For one-click setup, build, and launch, double-click `run_desktop.bat` in the project root. It checks that CMake, g++, and MinGW Make are available, configures the GUI, builds it, and opens the window.

Run these commands from the project root in PowerShell:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCHRONOMESH_BUILD_DESKTOP_UI=ON
cmake --build build --target ChronoMeshDesktop
.\build\ChronoMeshDesktop.exe
```

The project includes GLFW and Dear ImGui source files under `third_party/`, so the desktop UI build does not download or install these libraries on your computer.

The graphical layout uses a light theme, a left navigation menu, a dashboard, and separate task, graph, timeline, analytics, and benchmark pages. Add tasks and dependencies from **Tasks**. On **Dependency Graph**, click a node to inspect it and drag nodes to rearrange the view. Selecting a task bar on **Worker Timeline** selects the same task in the graph and inspector. Dependency cycles are rejected when they are added.

## Build the terminal interface

The existing terminal interface remains a separate target:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --target ChronoMeshUI
.\build\ChronoMeshUI.exe
```

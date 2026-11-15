# ChronoMesh UI

The UI is a C++17 terminal menu, so it does not need a browser, a JavaScript runtime, or extra libraries.

Build it from the project root using the commands in the main `Readme.md`:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --target ChronoMeshUI
.\build\ChronoMeshUI.exe
```

Task IDs start at 0 and increase as tasks are added. A dependency from task A to task B means A must finish before B can start. The menu shows the dependency graph, simulates task execution across workers, reports statistics, and benchmarks the scheduling routines.

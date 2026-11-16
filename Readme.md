
# ChronoMesh

A dependency-aware task scheduler that models tasks as a Directed Acyclic Graph (DAG), schedules them based on priority, and simulates parallel execution across multiple workers.

## Project Structure

```text
ChronoMesh/
├── include/             # Shared header files and interfaces
├── src/
│   ├── graph/           # Dependency graph and cycle detection
│   ├── scheduler/       # Topological sorting and scheduling
│   ├── execution/       # Worker management and task execution
│   └── main.cpp         # Existing graph-module menu (kept separate)
├── ui/
│   ├── src/             # UI implementation, visualization and app entry point
│   ├── assets/          # Images, icons and styles
│   └── README.md        # UI setup instructions
├── tests/               # Module-wise test cases
├── docs/                # Project documentation
├── CMakeLists.txt       # CMake build configuration
├── .gitignore
└── README.md
```

## Generate Folder Structure

Run this **single command** in PowerShell from the project root:

```powershell
New-Item -ItemType Directory -Force -Path "include","src/graph","src/scheduler","src/execution","ui/src","ui/assets","tests","docs"
```

This creates all required directories without overwriting existing files.

## Requirements

- C++17 compatible compiler (MinGW-w64 recommended)
- CMake 3.16 or later
- MinGW Make (`mingw32-make`)
- Git

## Setup

Clone the repository:

```bash
git clone https://github.com/Janmejai-Pandey/ChronoMesh.git
cd ChronoMesh
```

Verify installations:

```powershell
g++ --version
mingw32-make --version
cmake --version
```

Ensure all three tools are available in your system PATH.

## Build Instructions (Windows + MinGW)

**1. Configure the project**

```powershell
cmake -S . -B build -G "MinGW Makefiles"
```

**2. Build the project**

```powershell
cmake --build build
```

**3. Run the executable**

```powershell
.\build\ChronoMesh.exe
```

## CMake Commands

| Command | Purpose |
|---|---|
| `cmake -S . -B build -G "MinGW Makefiles"` | Configure project |
| `cmake --build build` | Compile project |
| `cmake --build build --clean-first` | Clean and rebuild |
| `Remove-Item -Recurse -Force build` | Remove build directory |

**Note:**
- Configure initially before building.
- Reconfigure when CMake configuration or source-file lists change.
- Delete the existing `build/` directory before switching generators.
- Never commit the generated `build/` directory.

## Team Modules

| Module | Responsibility |
|---|---|
| Dependency Graph | DAG construction and validation |
| Scheduling Engine | Topological sorting and priority scheduling |
| Execution Engine | Worker management and execution simulation |
| UI & Analytics | Visualization, statistics and integration |

## Development Workflow

- Each member works on a separate feature branch.
- Finalize shared interfaces before implementation.
- Keep `main` stable and working.

@echo off
setlocal
cd /d "%~dp0"

echo ==============================
echo ChronoMesh Build Started
echo ==============================
echo.

echo [1/3] Checking required files...
echo.

if not exist "CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found.
    goto :failed
)

if not exist "ui\src\desktop_main.cpp" (
    echo ERROR: ChronoMesh desktop UI source was not found.
    goto :failed
)

if not exist "src\graph\Graph.cpp" (
    echo ERROR: Graph module source was not found.
    goto :failed
)

if not exist "src\scheduler\Scheduler.cpp" (
    echo ERROR: Scheduler module source was not found.
    goto :failed
)

if not exist "src\execution\Execution.cpp" (
    echo ERROR: Execution module source was not found.
    goto :failed
)

if not exist "src\execution\Analytics.cpp" (
    echo ERROR: Analytics module source was not found.
    goto :failed
)

if not exist "src\execution\Benchmark.cpp" (
    echo ERROR: Benchmark module source was not found.
    goto :failed
)

if not exist "ui\src\Visualization.cpp" (
    echo ERROR: Visualization module source was not found.
    goto :failed
)

if not exist "third_party\imgui\imgui.cpp" (
    echo ERROR: Dear ImGui files were not found in third_party\imgui.
    goto :failed
)

if not exist "third_party\imgui\backends\imgui_impl_glfw.cpp" (
    echo ERROR: Dear ImGui GLFW backend was not found.
    goto :failed
)

if not exist "third_party\imgui\backends\imgui_impl_opengl3.cpp" (
    echo ERROR: Dear ImGui OpenGL backend was not found.
    goto :failed
)

if not exist "third_party\glfw\CMakeLists.txt" (
    echo ERROR: GLFW source files were not found in third_party\glfw.
    goto :failed
)

echo Checking build tools...

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake was not found in PATH.
    echo Install CMake or add its bin folder to PATH, then try again.
    goto :failed
)

where g++ >nul 2>nul
if errorlevel 1 (
    echo ERROR: g++ was not found in PATH.
    echo Install MinGW-w64 or add its bin folder to PATH, then try again.
    goto :failed
)

where gcc >nul 2>nul
if errorlevel 1 (
    echo ERROR: gcc was not found in PATH.
    echo GLFW needs a C compiler. Install MinGW-w64 or add its bin folder to PATH.
    goto :failed
)

where mingw32-make >nul 2>nul
if errorlevel 1 (
    echo ERROR: mingw32-make was not found in PATH.
    echo Install MinGW Make or add its bin folder to PATH, then try again.
    goto :failed
)

echo All required files and build tools were found.
echo.

echo [2/3] Compiling project...
echo.

cmake -S . -B build-desktop -G "MinGW Makefiles" -DCHRONOMESH_BUILD_DESKTOP_UI=ON
if errorlevel 1 (
    echo.
    echo BUILD FAILED while configuring ChronoMesh.
    echo Please check the error messages above.
    goto :failed
)

cmake --build build-desktop --target ChronoMeshDesktop --parallel 4
if errorlevel 1 (
    echo.
    echo BUILD FAILED while compiling ChronoMesh.
    echo Please check the error messages above.
    goto :failed
)

if not exist "build-desktop\ChronoMeshDesktop.exe" (
    echo.
    echo ERROR: ChronoMeshDesktop.exe was not created.
    goto :failed
)

echo.
echo Build successful.
echo.

echo [3/3] Starting ChronoMesh...
echo.

"build-desktop\ChronoMeshDesktop.exe"

echo.
echo ChronoMesh closed.
pause
exit /b 0

:failed
echo.
echo ChronoMesh could not be started. Read the error above.
pause
exit /b 1

# Bundled graphics libraries

The desktop UI builds these libraries from the copies kept in this folder:

- Dear ImGui 1.92.8, including its official GLFW and OpenGL3 backends. License: `imgui/LICENSE.txt`.
- GLFW 3.4. License: `glfw/LICENSE.md`.

CMake does not fetch or install these dependencies. The application-specific code remains under `ui/src/`.

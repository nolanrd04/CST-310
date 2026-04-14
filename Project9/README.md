# Project 9 - OpenGL Scene Renderer

This project is a C++ OpenGL scene that renders:
- an 8x8 checkerboard floor
- a lit cube
- an imported cylinder model (`cylinder.obj`)
- an imported sphere model (`sphere.obj`)

It uses GLFW/GLEW for context + OpenGL loading, GLM for math, SOIL for texture loading helpers, and Assimp for model loading.

## Controls

- `Esc`: Close the window
- `R`: Reset camera position and orientation
- `Left/Right`: Translate camera left/right
- `Up/Down`: Translate camera up/down
- `Shift + Up/Down`: Move camera forward/backward
- `Shift + ,` and `Shift + .`: Roll camera +/-
- `Ctrl + Down/Up`: Pitch camera +/-
- `Ctrl + Right/Left`: Yaw camera +/-

## Run

A prebuilt executable is included:

```bash
./run
```

## Build Notes

There is no Makefile/CMake file in this folder. To rebuild, compile `main.cpp` and link OpenGL plus project dependencies (`GLEW`, `GLFW`, `SOIL`, `Assimp`, and `GLM` headers).  
Link flags can vary by OS/distribution.

## Project Layout

- `main.cpp`: App entry, render loop, scene setup, object transforms, and key input dispatch
- `Camera.h`: Camera state, movement handling, Euler-angle updates, and view-matrix logic
- `Model.h`: Assimp model loading and node/mesh traversal
- `Mesh.h`: GPU buffer setup (VAO/VBO/EBO) and mesh draw calls
- `shader.h`: Shader file loading, compile, link, and program use
- `*.vs` / `*.frag`: Vertex/fragment shaders for each object type

## Key Logic Hotspots

- Main loop and draw orchestration: `main.cpp` (`main`, render loop)
- Input handling: `main.cpp` (`key_callback`, `do_movement`)
- Camera math: `Camera.h` (`ProcessKeyboard`, `updateCameraVectors`)
- Model parsing and mesh extraction: `Model.h` (`loadModel`, `processNode`, `processMesh`)
- Mesh render path: `Mesh.h` (`setupMesh`, `Draw`)
- Lighting computation: `checkerboard.frag`, `cube.frag`, `cylinder.frag`, `sphere.frag`

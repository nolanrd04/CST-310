# Color Cube Flyby - Compilation and Running Guide

## Prerequisites (Linux/Ubuntu)

Install required OpenGL libraries:

```bash
sudo apt-get install libglew-dev libglm-dev freeglut3-dev libgl1-mesa-dev libx11-dev
```

## Compilation

### Option 1: Direct g++ Compilation

```bash
g++ -o ColorCubeFlyby ColorCubeFlyby.cpp -lGL -lGLU -lGLEW -lglut -lm
```

### Option 2: Using CMake

Create a `CMakeLists.txt` in the project directory:

```cmake
cmake_minimum_required(VERSION 3.10)
project(ColorCubeFlyby)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(GLUT REQUIRED)

add_executable(ColorCubeFlyby ColorCubeFlyby.cpp)

target_link_libraries(ColorCubeFlyby
    OpenGL::GL
    OpenGL::GLU
    GLEW::GLEW
    GLUT::GLUT
    m
)
```

Then build:

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./ColorCubeFlyby
```

The program will open a window displaying an RGB color cube with the camera flying around it in a circular orbit while tumbling. The cube has:
- Black (0,0,0) at the origin
- White (1,1,1) at the opposite corner
- Colors interpolated smoothly across faces using the fixed-function OpenGL pipeline

## Controls

The program runs automatically with no interactive controls. Close the window to exit.
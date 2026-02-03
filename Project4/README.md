# OpenGL 3D Scene Example

This example demonstrates fundamental OpenGL concepts for rendering 3D scenes with primitives and camera control.

## What This Program Demonstrates

### Core Concepts
- **3D Primitives**: Cubes, spheres, cones, cylinders, torus, teapot, icosahedron
- **Camera System**: First-person camera with `gluLookAt()`
- **Lighting**: Ambient, diffuse, and specular lighting with `GL_LIGHT0`
- **Materials**: Setting surface properties that interact with light
- **Transformations**: `glTranslatef()`, `glRotatef()`, `glScalef()`
- **Matrix Stack**: Using `glPushMatrix()` / `glPopMatrix()` to isolate transforms

### Controls
| Key | Action |
|-----|--------|
| W/S | Move forward/backward |
| A/D | Strafe left/right |
| Q/E | Move up/down |
| Arrow Keys | Look around |
| R | Reset camera position |
| ESC | Exit program |

## Prerequisites (Linux)

Install the required development libraries:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libsoil-dev
```

### Fedora/RHEL
```bash
sudo dnf install gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel SOIL-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel freeglut mesa glu soil
```

## Compilation

### Simple Compilation
```bash
g++ -o example RLRender.cpp -lGL -lGLU -lglut -lSOIL -lm
```

### With Debugging Symbols
```bash
g++ -g -o example RLRender.cpp -lGL -lGLU -lglut -lSOIL -lm
```

### With Optimizations (for release)
```bash
g++ -O2 -o example RLRender.cpp -lGL -lGLU -lglut -lSOIL -lm
```

### Explanation of Flags
| Flag | Purpose |
|------|---------|
| `-o example` | Name the output executable "example" |
| `-lGL` | Link OpenGL library |
| `-lGLU` | Link OpenGL Utility library (provides gluLookAt, gluPerspective, etc.) |
| `-lglut` | Link GLUT library (window management, input handling) |
| `-lSOIL` | Link SOIL library (texture/image loading) |
| `-lm` | Link math library (for sin, cos, etc.) |

## Running the Program

```bash
./example
```

## Code Structure Overview

```
main()
  |
  +-- glutInit()              # Initialize GLUT
  +-- glutInitDisplayMode()   # Set up double buffering, RGB, depth
  +-- glutCreateWindow()      # Create the window
  |
  +-- Callbacks registered:
  |     +-- display()         # Called every frame to render
  |     +-- reshape()         # Called when window resizes
  |     +-- keyboard()        # Called for key presses
  |     +-- specialKeys()     # Called for arrow keys
  |
  +-- init()                  # One-time OpenGL setup
  +-- glutMainLoop()          # Enter event loop (never returns)
```

## Key OpenGL Concepts Explained

### 1. The Rendering Pipeline

```
Vertices --> Transformations --> Lighting --> Rasterization --> Pixels
             (ModelView +        (per-vertex   (converts to
              Projection)         calculation)   fragments)
```

### 2. Matrix Modes

OpenGL uses different matrices for different purposes:

```cpp
glMatrixMode(GL_PROJECTION);  // For camera lens (perspective/ortho)
glMatrixMode(GL_MODELVIEW);   // For camera position + object transforms
```

### 3. Transformation Order

Transformations are applied in **reverse order** of how they're written:

```cpp
glTranslatef(5, 0, 0);    // Applied SECOND: move to position
glRotatef(45, 0, 1, 0);   // Applied FIRST: rotate object
drawCube();
```

This means: rotate the cube, THEN move it to (5, 0, 0).

### 4. Matrix Stack (glPushMatrix / glPopMatrix)

Used to isolate transformations for each object:

```cpp
glPushMatrix();           // Save current state
    glTranslatef(...);    // Transform for this object only
    drawObject();
glPopMatrix();            // Restore state - transform is gone

// Next object starts fresh
```

### 5. gluLookAt Explained

```cpp
gluLookAt(
    eyeX, eyeY, eyeZ,       // Where the camera IS
    centerX, centerY, centerZ,  // Where the camera LOOKS
    upX, upY, upZ           // Which direction is "up"
);
```

### 6. Lighting Components

| Component | Description | Example |
|-----------|-------------|---------|
| Ambient | Background light everywhere | Room's general brightness |
| Diffuse | Directional light, creates shading | Sunlight on a surface |
| Specular | Shiny highlights | Glare on a polished surface |

### 7. Normal Vectors

Normals are perpendicular vectors that tell OpenGL which way a surface faces. They're essential for lighting:

```cpp
glNormal3f(0.0f, 1.0f, 0.0f);  // This face points UP
glVertex3f(...);                // Vertices for this face
```

## Common Issues and Solutions

### Program won't compile - missing headers
Make sure freeglut is installed (see Prerequisites section).

### Black screen / no objects visible
- Check that `GL_DEPTH_TEST` is enabled
- Verify objects are within the camera's view frustum
- Make sure lighting is enabled and a light source exists

### Objects look flat (no shading)
- Enable lighting: `glEnable(GL_LIGHTING)`
- Enable at least one light: `glEnable(GL_LIGHT0)`
- Make sure normals are defined for your geometry

### Window appears but is white/blank
- Check that `glutSwapBuffers()` is called at the end of display()
- Verify `GLUT_DOUBLE` is in the display mode

## Extending This Example

Ideas for practice:
1. Add more objects to create a scene
2. Implement mouse look (use `glutMotionFunc`)
3. Add textures to objects
4. Create animation with `glutIdleFunc`
5. Add a second light source with different color
6. Draw your real-life scene using these primitives

## Further Reading

- OpenGL Red Book (official guide): https://www.opengl.org/documentation/red_book/
- GLUT documentation: https://www.opengl.org/resources/libraries/glut/
- NeHe OpenGL tutorials: https://nehe.gamedev.net/
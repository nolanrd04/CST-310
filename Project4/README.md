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
g++ -o rlrender RLRender.cpp -lGL -lGLU -lglut -lSOIL -lm
```

## Running the Program

```bash
./rlrender
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
# 3D Primitives OpenGL Framework

A simple framework for creating and rendering 3D primitive shapes with OpenGL.

## Features

- Easy-to-use primitive shape generation (Cube, Sphere, Cylinder, Plane)
- Simple object positioning, rotation, and scaling
- Phong lighting model for realistic shading
- Multiple objects in one scene
- Orbiting camera view

## Prerequisites

You need the following libraries installed:

- **OpenGL** - Graphics API
- **GLEW** - OpenGL Extension Wrangler
- **GLFW** - Window and input management
- **GLM** - OpenGL Mathematics library

### Installing Dependencies

#### Ubuntu/Debian:
```bash
make install-deps
```
or manually:
```bash
sudo apt-get update
sudo apt-get install build-essential libglew-dev libglfw3-dev libglm-dev
```

#### Fedora/RHEL:
```bash
make install-deps-fedora
```
or manually:
```bash
sudo dnf install gcc-c++ glew-devel glfw-devel glm-devel
```

#### Arch Linux:
```bash
make install-deps-arch
```
or manually:
```bash
sudo pacman -S base-devel glew glfw-x11 glm
```

#### macOS:
```bash
brew install glew glfw glm
```

## Compiling and Running

### Quick Start:
```bash
make run
```

### Step by Step:

1. **Compile the program:**
   ```bash
   make
   ```

2. **Run the program:**
   ```bash
   ./primitives_3d
   ```

### Other Build Options:

- **Debug build** (with debug symbols):
  ```bash
  make debug
  ./primitives_3d
  ```

- **Release build** (optimized):
  ```bash
  make release
  ./primitives_3d
  ```

- **Clean build files:**
  ```bash
  make clean
  ```

- **See all options:**
  ```bash
  make help
  ```

## Controls

- **ESC** - Exit the program
- The camera automatically orbits around the scene

## Usage Example

```cpp
// 1. Create meshes (do once)
Mesh sphereMesh = createSphere(1.0f, 32, 16);
Mesh cubeMesh = createCube(1.0f);

// 2. Create objects with position and color
Object3D ball(&sphereMesh, 
              glm::vec3(0.0f, 2.0f, 0.0f),  // Position
              glm::vec3(1.0f, 0.0f, 0.0f)); // Red color

// 3. Modify properties
ball.scale = glm::vec3(1.5f);           // Make it bigger
ball.rotation = glm::vec3(45.0f, 0.0f, 0.0f);  // Rotate

// 4. Add to scene
objects.push_back(ball);

// 5. In render loop, draw all objects
for (Object3D& obj : objects) {
    glm::mat4 model = obj.getModelMatrix();
    glUniformMatrix4fv(..., model);
    glUniform3fv(..., obj.color);
    obj.mesh->draw();
}
```

## Available Shapes

- `createPlane(width, depth, subdivW, subdivD)` - Flat ground plane
- `createCube(size)` - Box/cube
- `createSphere(radius, segments, rings)` - Sphere/ball
- `createCylinder(radius, height, segments)` - Cylinder/tube

## Project Structure

```
.
├── primitives_3d.cpp    # Main source file
├── Makefile             # Build configuration
└── README.md            # This file
```

## Troubleshooting

### "GL/glew.h: No such file or directory"
You need to install the GLEW development files:
```bash
make install-deps
```

### "undefined reference to 'glfwInit'"
Make sure you're linking with `-lglfw`. The Makefile handles this automatically.

### Black screen or no window appears
- Check that your graphics drivers support OpenGL 3.3 or higher
- Try updating your graphics drivers

### Window opens but nothing renders
- Verify OpenGL context creation succeeded (check console output)
- Ensure depth testing is enabled

## License

Free to use and modify for any purpose.

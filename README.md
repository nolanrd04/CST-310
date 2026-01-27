# 3D Curtain OpenGL Program

This program renders a 3D grey curtain with Phong lighting using modern OpenGL (3.3+).

## Features
- 3D vertical curtain mesh (2m wide x 3m tall)
- Phong lighting model (ambient, diffuse, specular)
- Smooth grey material
- Rotating animation to showcase the 3D effect
- Subdivided mesh for better lighting

## Requirements

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libglew-dev libglfw3-dev libglm-dev
```

### Fedora/RHEL:
```bash
sudo dnf install gcc-c++
sudo dnf install glew-devel glfw-devel glm-devel
```

## Compilation

Simply run:
```bash
make
```

Or manually compile:
```bash
g++ -std=c++11 -o curtain curtain.cpp -lGL -lGLEW -lglfw -lm
```

## Running

```bash
make run
```

Or directly:
```bash
./curtain
```

## Controls
- **ESC**: Exit the application

## How it Works

1. **Vertex Shader**: Transforms vertices and passes position/normal to fragment shader
2. **Fragment Shader**: Implements Phong lighting (ambient + diffuse + specular)
3. **Mesh Generation**: Creates a subdivided rectangle for smooth lighting
4. **Animation**: Gentle rotation around Y-axis to show 3D depth

## Customization

You can modify these parameters in `curtain.cpp`:

- **Curtain size**: Change `generateCurtainMesh(2.0f, 3.0f, ...)` parameters
- **Color**: Modify `objectColor(0.6f, 0.6f, 0.65f)` for different grey shades
- **Light position**: Adjust `lightPos(3.0f, 3.0f, 3.0f)`
- **Rotation speed**: Change the multiplier in `glfwGetTime() * 0.2f`
- **Subdivisions**: Increase `(20, 30)` for smoother lighting

## Troubleshooting

If you get linking errors, make sure all libraries are installed:
```bash
pkg-config --libs glfw3 glew
```

If the window doesn't appear, your GPU might not support OpenGL 3.3. Try changing the version in the code to 3.0 or 2.1.

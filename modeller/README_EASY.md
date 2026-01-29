# Easy 3D Shapes - OpenGL Framework

A super simple framework for creating 3D shapes with just one function call!

## Quick Start

```bash
make run
```

That's it! The program will compile and show you a demo scene with various colored shapes.

## Creating Your Own Scenes

Edit the `modeler.cpp` file and add your shapes in the `main()` function. It's incredibly easy:

```cpp
// Create a red cube at position (5, 2, 0)
makeCube(5, 2, 0,           // Position (x, y, z)
         0, 0, 0,           // Rotation (rotX, rotY, rotZ)
         1, 1, 1,           // Scale (scaleX, scaleY, scaleZ)
         1.0, 0.0, 0.0);    // Color (r, g, b) - RED

// Create a green sphere
makeSphere(0, 0, 0, 0, 0, 0, 1, 1, 1, 0.0, 1.0, 0.0);  // GREEN

// Or use defaults - this creates a grey cube at origin
makeCube();
```

## Available Functions

All functions follow the same pattern:

```cpp
makeCube(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
makeSphere(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
makeCylinder(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
makePlane(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
makeRectangle(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
```

### Parameter Order (Always the Same!)

1. **Position** (x, y, z) - Where the object is in 3D space
2. **Rotation** (rotX, rotY, rotZ) - Rotation in degrees around each axis
3. **Scale** (scaleX, scaleY, scaleZ) - Size multiplier (1.0 = normal, 2.0 = double size)
4. **Color** (r, g, b) - RGB values from 0.0 to 1.0

### Default Values

All parameters are optional! If you don't specify them, they default to:
- **Position**: 0, 0, 0 (origin)
- **Rotation**: 0, 0, 0 (no rotation)
- **Scale**: 1, 1, 1 (normal size)
- **Color**: 0.7, 0.7, 0.7 (grey)

## Shape-Specific Notes

### makeCube()
- Creates a 1x1x1 cube centered at the position
- Use scale to make it bigger or create rectangular boxes

```cpp
// Normal cube
makeCube(0, 0, 0);

// Wide flat box (2x wide, 0.5x tall, 1x deep)
makeCube(0, 0, 0, 0, 0, 0, 2, 0.5, 1);

// Rotating red cube
makeCube(-3, 0, 0, 0, 45, 0, 1, 1, 1, 1.0, 0.0, 0.0);
```

### makeSphere()
- Creates a sphere with diameter 1.0 (radius 0.5)
- Use different scales to create ellipsoids

```cpp
// Normal sphere
makeSphere(0, 0, 0);

// Tall ellipsoid (stretched vertically)
makeSphere(0, 2, 0, 0, 0, 0, 1, 2, 1);

// Green ball
makeSphere(0, 0, 0, 0, 0, 0, 1, 1, 1, 0.0, 1.0, 0.0);
```

### makeCylinder()
- Creates a cylinder aligned with the Y axis (vertical by default)
- Height = 1.0, radius = 0.5
- Rotate 90° around X or Z to make it horizontal

```cpp
// Vertical cylinder
makeCylinder(0, 0, 0);

// Horizontal cylinder (like a log)
makeCylinder(0, 0, 0, 0, 0, 90);

// Tall thin pillar
makeCylinder(0, 0, 0, 0, 0, 0, 0.5, 3, 0.5);

// Blue cylinder
makeCylinder(3, 0, 0, 0, 0, 0, 1, 1, 1, 0.2, 0.4, 1.0);
```

### makePlane()
- Creates a flat 1x1 square in the XZ plane (horizontal)
- Normal points up (Y axis)
- Great for ground/floors

```cpp
// Ground plane (large and white)
makePlane(0, -2, 0, 0, 0, 0, 10, 1, 10, 0.9, 0.9, 0.9);

// Vertical wall (rotate 90° around X)
makePlane(0, 0, -5, 90, 0, 0);

// Green grass
makePlane(0, 0, 0, 0, 0, 0, 5, 1, 3, 0.3, 0.8, 0.3);
```

### makeRectangle()
- Creates a flat 1x1 rectangle in the XY plane
- Faces toward you (positive Z direction)
- Great for billboards, signs, or UI elements

```cpp
// Simple rectangle facing camera
makeRectangle(0, 0, 0);

// Wide billboard
makeRectangle(0, 2, 0, 0, 0, 0, 3, 2, 1);

// Yellow sign rotated to face sideways
makeRectangle(-2, 1, 0, 0, 90, 0, 1, 2, 1, 1.0, 1.0, 0.0);
```

## Common Colors

Here are RGB values for common colors (use for the r, g, b parameters):

| Color   | r   | g   | b   |
|---------|-----|-----|-----|
| Red     | 1.0 | 0.0 | 0.0 |
| Green   | 0.0 | 1.0 | 0.0 |
| Blue    | 0.0 | 0.0 | 1.0 |
| Yellow  | 1.0 | 1.0 | 0.0 |
| Cyan    | 0.0 | 1.0 | 1.0 |
| Magenta | 1.0 | 0.0 | 1.0 |
| Orange  | 1.0 | 0.5 | 0.0 |
| Purple  | 0.5 | 0.0 | 1.0 |
| Pink    | 1.0 | 0.5 | 0.8 |
| White   | 1.0 | 1.0 | 1.0 |
| Black   | 0.0 | 0.0 | 0.0 |
| Grey    | 0.5 | 0.5 | 0.5 |

## Complete Example

```cpp
// Create a scene with a ground, some colorful shapes, and a sun

// Ground (large white plane below everything)
makePlane(0, -2, 0, 0, 0, 0, 20, 1, 20, 0.9, 0.9, 0.9);

// Red house (cube body + triangular roof could be multiple rectangles)
makeCube(0, 0, 0, 0, 0, 0, 3, 2, 3, 0.8, 0.2, 0.2);

// Yellow sun (sphere in the sky)
makeSphere(-5, 8, -5, 0, 0, 0, 2, 2, 2, 1.0, 1.0, 0.0);

// Blue pool (flat plane)
makePlane(5, -1.9, 0, 0, 0, 0, 4, 1, 3, 0.2, 0.5, 1.0);

// Tree trunk (brown cylinder)
makeCylinder(-6, -1, 2, 0, 0, 0, 0.5, 3, 0.5, 0.6, 0.3, 0.1);

// Tree leaves (green sphere on top)
makeSphere(-6, 1.5, 2, 0, 0, 0, 2, 2, 2, 0.2, 0.8, 0.2);
```

## Installation

### Ubuntu/Debian:
```bash
make install-deps
make run
```

### Other Systems:
See the main README.md for other operating systems.

## Compiling

```bash
make          # Compile
make run      # Compile and run
make clean    # Remove compiled files
make debug    # Compile with debug symbols
```

## Tips

1. **Start Simple**: Begin with just position and color:
   ```cpp
   makeCube(x, y, z, 0, 0, 0, 1, 1, 1, r, g, b);
   ```

2. **Use Defaults**: You can skip parameters you don't need:
   ```cpp
   makeCube();  // Everything at default
   makeCube(5, 2, 0);  // Just position, rest default
   ```

3. **Build Incrementally**: Add one shape at a time and run to see the result

4. **Coordinate System**:
   - X: Left (-) to Right (+)
   - Y: Down (-) to Up (+)
   - Z: Away (-) to Toward you (+)

5. **Rotation**:
   - rotX: Pitch (nod yes)
   - rotY: Yaw (shake head no)
   - rotZ: Roll (tilt head)

6. **Scale**:
   - 0.5 = half size
   - 1.0 = normal size
   - 2.0 = double size
   - Use different values for each axis to stretch shapes

## Controls

- **ESC** - Exit program
- Camera automatically orbits the scene

## Troubleshooting

**Shapes not appearing?**
- Check if they're behind the camera (try negative Z values)
- Make sure they're not too far from origin

**Wrong colors?**
- RGB values should be between 0.0 and 1.0 (not 0-255)

**Can't compile?**
- Run `make install-deps` to install required libraries

## What's Next?

- Try creating a simple house, car, or robot
- Build a solar system with planets
- Create a forest scene with trees
- Make a city with buildings

Have fun creating!

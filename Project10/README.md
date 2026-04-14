# Project 10: Advanced Texture Mapping with Parallax and Bump Mapping

## Overview

This project demonstrates advanced texture mapping techniques in OpenGL, featuring **parallax mapping** and **bump mapping** applied to various 3D objects. The scene includes a checkerboard floor and two 3D models (cylinder and sphere) with bump mapping.

## Theoretical Background

### Bump Mapping
Bump mapping perturbs surface normals based on a texture to simulate surface details without increasing geometric complexity. This is particularly effective for creating the appearance of bumpy or rough surfaces.

**Key Concept**: Instead of modifying geometry, bump maps modify how light interacts with the surface by altering its normal vectors.

### Tangent Space
Bump mapping relies on tangent space calculations. The **TBN matrix** (Tangent-Bitangent-Normal) transforms vectors from world space to tangent space, allowing normal maps to be correctly applied regardless of the surface's orientation.

### Phong Lighting Model
The project uses the Phong shading model, which consists of three components:
- **Ambient**: Base lighting that simulates scattered light
- **Diffuse**: Lambertian reflection based on surface normal and light direction
- **Specular**: Shiny highlights based on the reflection of light toward the viewer

## Mathematical Functions and Models

### 1. **Camera Euler Angles (Camera.h)**

**Function**: `updateCameraVectors()`

**Mathematical Model**:
```
front.x = cos(Yaw) * cos(Pitch)
front.y = sin(Pitch)
front.z = sin(Yaw) * cos(Pitch)
Right = normalize(cross(Front, WorldUp))
Up = normalize(cross(Right, Front))
```

**Reasoning**: Euler angles (Yaw, Pitch, Roll) provide intuitive camera control by rotating around three axes. The front vector is computed using trigonometric functions to convert spherical coordinates to Cartesian coordinates.

**Example**: With Yaw = -90°, Pitch = 0°, the front vector points along the negative z-axis, creating a standard "looking forward" orientation.

---

### 2. **Tangent and Bitangent Calculation (Model.h)**

**Function**: `calculateTangentsBitangents()`

**Mathematical Model**:
For each triangle with vertices at positions v1, v2, v3 and texture coordinates uv1, uv2, uv3:

```
e1 = v2 - v1
e2 = v3 - v1
duv1 = uv2 - uv1
duv2 = uv3 - uv1

f = 1.0 / (duv1.x * duv2.y - duv2.x * duv1.y)

tangent = f * (duv2.y * e1 - duv1.y * e2)
bitangent = f * (-duv2.x * e1 + duv1.x * e2)
```

**Reasoning**: These calculations compute the directions along the surface in texture space. Tangent represents the direction of increasing U texture coordinate, and bitangent represents the direction of increasing V coordinate. This forms a coordinate system for interpreting normal and height maps.

**Example**: On a textured plane, the tangent points right and the bitangent points down in world space, aligned with the texture's U and V axes.

---

### 3. **Phong Lighting Model**

**Applied in**: All fragment shaders (checkerboard.frag, sphere.frag, cylinder.frag)

**Mathematical Model**:
```
lightDir = normalize(lightPos - FragPos)
viewDir = normalize(viewPos - FragPos)

// Ambient
ambient = ambientStrength * lightColor

// Diffuse (Lambertian)
diffuse = max(dot(normal, lightDir), 0.0) * lightColor

// Specular (Blinn-Phong variant)
reflectDir = reflect(-lightDir, normal)
specular = pow(max(dot(viewDir, reflectDir), 0.0), shininess) * lightColor

finalColor = (ambient + diffuse + specular) * textureColor
```

**Reasoning**: 
- Ambient provides base illumination
- Diffuse simulates rough surface interaction (dot product of normal and light direction)
- Specular creates highlights based on view angle (higher powers create sharper highlights)

**Example**: A white light at position (1, 1, -2) shining on a textured sphere creates:
- Ambient component from the base texture color
- Bright diffuse on front-facing surfaces
- White specular highlights where the viewer's angle aligns with reflection

---

### 4. **View Matrix Calculation (main.cpp)**

**Function**: `glm::lookAt(Position, Position + Front, Up)`

**Mathematical Model**: 
The lookAt function creates a view matrix that transforms world coordinates into camera-relative coordinates:
```
-z axis = normalize(Position - (Position + Front)) = -Front
x axis = normalize(cross(Front, Up))
y axis = cross(-z, x)
```

**Reasoning**: This creates a coordinate system where the camera is at the origin, looking down the -z axis. The view matrix transforms all world geometry into this camera space.

**Example**: A camera at (0, 0, 3) looking toward (0, 0, 0) with up as (0, 1, 0) creates a standard view where objects at z = 0 appear centered.

---

## Key Aesthetic and Programming Decisions

1. **Checkerboard Floor**: Uses a simple alternating color pattern based on grid position to provide visual context for the scene.

2. **Tangent Space Transformation**: All lighting is computed in tangent space for the sphere and cylinder to correctly apply bump mapping regardless of surface orientation.

3. **Fallback Textures**: White diffuse and gray height map fallbacks ensure the scene renders even if texture files are missing.

4. **TBN Matrix**: Computed in the vertex shader and passed to fragment shader for efficient per-fragment normal transformation.

## Compilation

```bash
g++ -o project10 main.cpp -lGL -lGLU -lGLEW -lglfw -lSOIL -lassimp -lm
```

## Controls

- **Arrow Keys / WASD**: Move camera
- **Arrow Keys (Up/Down)**: Rotate pitch
- **Arrow Keys (Left/Right)**: Rotate yaw
- **Numpad**: Roll rotation

## Dependencies

- OpenGL 3.3+
- GLEW (OpenGL Extension Wrangler)
- GLFW (window management)
- GLM (mathematics)
- SOIL (texture loading)
- ASSIMP (model loading)
# Project 10: Environment, Parallax, and Bump Mapping

## Overview

Scene contains checkerboard floor, cube, cylinder, sphere.

Mapping assignment:
- **Sphere**: environment mapping with cubemap (`posx/negx/posy/negy/posz/negz`).
- **Cube**: **parallax mapping only** (uses `Bump-Picture.jpg` + `Bump-Map.jpg`).
- **Cylinder**: **bump mapping only** (uses `Bump-Picture.jpg` + `Bump-Map.jpg`).

## Current Shader Behavior

### Sphere (`sphere.vs`, `sphere.frag`)
- Uses cubemap reflection when viewed from outside.
- Uses direct cubemap lookup when camera is inside sphere.
- Inside logic avoids upside-down/inverted interior look.

### Cube (`cube.vs`, `cube.frag`)
- Uses height map for UV displacement (parallax effect).
- Does **not** perturb normals from height map (no bump mapping path).
- Parallax strength set higher for visibility.

### Cylinder (`cylinder.vs`, `cylinder.frag`)
- Uses height map to derive perturbed tangent-space normal (bump mapping).
- Uses tiled UVs (`uvScale = vec2(3.0, 2.0)`) so texture repeats around cylinder.
- Bump strength increased for more obvious relief.

## Texture Inputs

- `Bump-Picture.jpg` → diffuse/albedo source.
- `Bump-Map.jpg` → height source (parallax on cube, bump on cylinder).
- `posx.jpg`, `negx.jpg`, `posy.jpg`, `negy.jpg`, `posz.jpg`, `negz.jpg` → cubemap faces.

2D textures use `GL_REPEAT`, so repeating works.  
If source image not seamless, seam can still be visible.

## Build

```bash
g++ -o project10 main.cpp -lGL -lGLU -lGLEW -lglfw -lSOIL -lassimp -lm
```

## Run

```bash
./project10
```

## Controls

Default:
- `Left` / `Right` / `Up` / `Down`: translate camera.
- `R`: reset camera.

With `Shift` held:
- `Up` / `Down`: forward / backward.
- `,` / `.`: roll up / roll down.

With `Ctrl` held:
- `Left` / `Right`: yaw adjust.
- `Up` / `Down`: pitch adjust.

## Main Files

- `main.cpp` — scene setup, texture loading, draw order, uniforms.
- `cube.vs`, `cube.frag` — parallax-only cube shader path.
- `cylinder.vs`, `cylinder.frag` — bump-only cylinder shader path.
- `sphere.vs`, `sphere.frag` — cubemap environment sphere shader path.
- `Model.h`, `Mesh.h` — model loading + tangent/bitangent setup.

# Geometry Dash OpenGL Platformer

A side-scrolling platformer game built with OpenGL, inspired by Geometry Dash.

## Installation (Ubuntu/Debian)

```bash
sudo apt-get install libglew-dev libglm-dev freeglut3-dev libgl1-mesa-dev libx11-dev
```

## Compile

```bash
cd Project8
g++ -o Project8 main.cpp renderer.cpp player.cpp obstacle.cpp level.cpp -lGL -lGLU -lGLEW -lglut -lm
```

## Run

```bash
./Project8
```

## Controls

- **SPACE** — Start game or Jump
- **R** — Restart
- **ESC** — Exit
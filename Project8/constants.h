#ifndef CONSTANTS_H
#define CONSTANTS_H

// Window dimensions
const int WINDOW_W = 1200;
const int WINDOW_H = 800;

// World coordinates
const float GROUND_Y = 120.0f;           // top of ground strip
const float PLAYER_SCREEN_X = 200.0f;   // fixed screen X where player appears

// Physics
const float GRAVITY = -2530.0f;          // units/sec²
const float JUMP_VEL = 900.0f;           // units/sec upward
const float PLAYER_WIDTH = 60.0f;
const float PLAYER_HEIGHT = 60.0f;
const float ROTATION_SPEED = -270.f;     // degrees/sec during jump

// Grid system (Geometry Dash-style editor grid)
const float GRID_CELL_SIZE = 60.0f;      // 1 cell = player width/height

// Obstacle dimensions (grid-derived)
const float SPIKE_W                = 1.0f * GRID_CELL_SIZE;   // 60px
const float SPIKE_H                = 1.0f * GRID_CELL_SIZE;   // 60px (renderer uses equilateral formula)
const float PLATFORM_H             = 0.5f * GRID_CELL_SIZE;   // 30px thin ledge

// Level layout (grid-derived)
const float LEVEL_LENGTH           = 150.0f * GRID_CELL_SIZE; // 9000px — unchanged
const float RUNWAY_CELLS           = 10.0f;   // clear cells before first obstacle
const float BUFFER_CELLS           = 10.0f;   // clear cells at end

// Inter-obstacle gap range (in cells)
const float GAP_MIN_CELLS          = 4.0f;    // min gap between obstacle groups
const float GAP_RANGE_CELLS        = 4.0f;    // rand range (4–7 cells = 240–420px)

// Spike cluster
const float SPIKE_INTRA_GAP        = 0.0f;    // spikes in a cluster touch (no gap)

// Platform spike options
const bool ENABLE_PLATFORM_TOP_SPIKES = true;
const bool ENABLE_PLATFORM_UNDER_SPIKES = true;

// Platform sizing (in cells)
const float PLATFORM_MIN_CELLS     = 1.0f;    // min 1 cells wide = 60px
const float PLATFORM_RANGE_CELLS   = 3.0f;    // rand range (1–3 cells = 60–180px)

// Platform elevation above GROUND_Y (in cells)
const float PLAT_ELEV_MIN_CELLS    = 1.0f;    // 1 cell = 60px above ground
const float PLAT_ELEV_RANGE_CELLS  = 3.0f;    // rand range (1–3 cells = 60–180px)

// Finish line
const float FINISH_W               = 2.0f * GRID_CELL_SIZE;   // 120px
const float FINISH_H               = 3.0f * GRID_CELL_SIZE;   // 180px

// Level & Scroll
const float SCROLL_SPEED = 450.0f;       // units/sec (world moves left)
const float DT = 0.016f;                 // fixed timestep (16ms)

// Colors (RGB 0-1)
const float PLAYER_R = 1.0f, PLAYER_G = 1.0f, PLAYER_B = 0.0f;  // yellow
const float PLATFORM_R = 0.7f, PLATFORM_G = 0.7f, PLATFORM_B = 0.7f;  // gray
const float FINISH_R = 1.0f, FINISH_G = 0.85f, FINISH_B = 0.0f; // gold
const float GROUND_BOT_R = 0.2f, GROUND_BOT_G = 0.15f, GROUND_BOT_B = 0.1f;   // dark base

// Spike colors
const float SPIKE_FILL_R = 0.0f, SPIKE_FILL_G = 0.0f, SPIKE_FILL_B = 0.0f;   // black fill
const float SPIKE_OUTLINE_R = 1.0f, SPIKE_OUTLINE_G = 1.0f, SPIKE_OUTLINE_B = 1.0f; // white outline

// Color animation parameters (hue rotation)
const float COLOR_CYCLE_SPEED = 35.0f;  // seconds for one full hue rotation (very slow)

#endif

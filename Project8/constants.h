#ifndef CONSTANTS_H
#define CONSTANTS_H

// Window dimensions
const int WINDOW_W = 1200;
const int WINDOW_H = 800;

// World coordinates
const float GROUND_Y = 120.0f;           // top of ground strip
const float PLAYER_SCREEN_X = 200.0f;   // fixed screen X where player appears

// Physics
const float GRAVITY = -1800.0f;          // units/sec²
const float JUMP_VEL = 750.0f;           // units/sec upward
const float PLAYER_WIDTH = 60.0f;
const float PLAYER_HEIGHT = 60.0f;

// Level & Scroll
const float SCROLL_SPEED = 350.0f;       // units/sec (world moves left)
const float LEVEL_LENGTH = 9000.0f;      // total level width
const float DT = 0.016f;                 // fixed timestep (16ms)

// Colors (RGB 0-1)
const float PLAYER_R = 0.2f, PLAYER_G = 0.6f, PLAYER_B = 1.0f;  // blue
const float PLATFORM_R = 0.7f, PLATFORM_G = 0.7f, PLATFORM_B = 0.7f;  // gray
const float FINISH_R = 1.0f, FINISH_G = 0.85f, FINISH_B = 0.0f; // gold
const float GROUND_BOT_R = 0.2f, GROUND_BOT_G = 0.15f, GROUND_BOT_B = 0.1f;   // dark base

// Spike colors
const float SPIKE_FILL_R = 0.0f, SPIKE_FILL_G = 0.0f, SPIKE_FILL_B = 0.0f;   // black fill
const float SPIKE_OUTLINE_R = 1.0f, SPIKE_OUTLINE_G = 1.0f, SPIKE_OUTLINE_B = 1.0f; // white outline

// Color animation parameters (hue rotation)
const float COLOR_CYCLE_SPEED = 30.0f;  // seconds for one full hue rotation (very slow)

#endif
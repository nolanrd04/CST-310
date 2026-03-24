#include "player.h"
#include "constants.h"
#include <cmath>

Player::Player() : worldX(0), y(GROUND_Y), velY(0), onGround(true), w(PLAYER_WIDTH), h(PLAYER_HEIGHT), rotation(0), timeInAir(0), prevY(GROUND_Y) {
}

void Player::reset() {
    worldX = 0;
    y = GROUND_Y;
    velY = 0;
    onGround = true;
    rotation = 0;
    timeInAir = 0;
    prevY = GROUND_Y;
}

void Player::update(float dt, float groundY) {
    // Save previous y position before updating (for collision checks)
    prevY = y;

    // Apply gravity
    velY += GRAVITY * dt;

    // Update position
    y += velY * dt;

    // Check ground collision
    if (y <= groundY) {
        y = groundY;
        velY = 0;
        onGround = true;
        // Snap rotation to nearest 90° when landing
        rotation = snapRotationTo90(rotation);
    } else if (velY != 0.0f) {
        // Rotate whenever falling or jumping (experiencing gravity/movement)
        timeInAir += dt;
        rotation += ROTATION_SPEED * dt;
        // Keep rotation in 0-360 range
        rotation = std::fmod(rotation, 360.0f);
        if (rotation < 0.0f) rotation += 360.0f;
    }

    // Track distance traveled (for win check)
    worldX += SCROLL_SPEED * dt;
}

void Player::jump() {
    if (onGround) {
        velY = JUMP_VEL;
        onGround = false;
        timeInAir = 0.0f;
    }
}

// Snap rotation to nearest 90° increment (0°, 90°, 180°, 270°)
float snapRotationTo90(float rotation) {
    return std::round(rotation / 90.0f) * 90.0f;
}

float Player::screenX() const {
    return PLAYER_SCREEN_X;
}
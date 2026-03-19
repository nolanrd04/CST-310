#include "player.h"
#include "constants.h"

Player::Player() : worldX(0), y(GROUND_Y), velY(0), onGround(true), w(PLAYER_WIDTH), h(PLAYER_HEIGHT) {
}

void Player::reset() {
    worldX = 0;
    y = GROUND_Y;
    velY = 0;
    onGround = true;
}

void Player::update(float dt, float groundY) {
    // Apply gravity
    velY += GRAVITY * dt;

    // Update position
    y += velY * dt;

    // Check ground collision
    if (y <= groundY) {
        y = groundY;
        velY = 0;
        onGround = true;
    } else {
        onGround = false;
    }

    // Track distance traveled (for win check)
    worldX += SCROLL_SPEED * dt;
}

void Player::jump() {
    if (onGround) {
        velY = JUMP_VEL;
        onGround = false;
    }
}

float Player::screenX() const {
    return PLAYER_SCREEN_X;
}
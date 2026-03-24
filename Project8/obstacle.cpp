#include "obstacle.h"
#include "player.h"
#include "constants.h"

Obstacle::Obstacle(ObstacleType t, float x, float posY, float width, float height, glm::vec3 col)
    : type(t), worldX(x), y(posY), w(width), h(height), color(col) {
}

// Simple AABB overlap test
static bool aabbOverlap(float ax1, float ay1, float aw, float ah,
                        float bx1, float by1, float bw, float bh) {
    return ax1 < bx1 + bw &&
           ax1 + aw > bx1 &&
           ay1 < by1 + bh &&
           ay1 + ah > by1;
}

bool checkCollisionWithSpike(const Player& p, const Obstacle& spike, float camX) {
    float spikeScreenX = spike.worldX - camX;
    float playerScreenX = p.screenX();

    // Use a smaller square hitbox inside the triangle (centered horizontally at base)
    float sqSize = spike.w * 0.55f;  // Square is 55% of triangle width
    float sqX = spikeScreenX + (spike.w - sqSize) / 2.0f;  // Center horizontally
    float sqY = spike.y;  // Positioned at triangle base

    // AABB overlap: player vs square hitbox inside spike
    return aabbOverlap(playerScreenX, p.y, p.w, p.h,
                      sqX, sqY, sqSize, sqSize);
}

bool checkCollisionWithPlatform(const Player& p, const Obstacle& platform, float camX) {
    float platScreenX = platform.worldX - camX;
    float playerScreenX = p.screenX();

    // Only collide with top of platform (player above and falling)
    // Platform top is at platform.y + platform.h
    float platTop = platform.y + platform.h;

    // Check if player is roughly above platform and at the right Y level
    // Player bottom should be near platform top
    if (playerScreenX + p.w > platScreenX && playerScreenX < platScreenX + platform.w) {
        // Horizontal overlap with platform
        if (p.y <= platTop && p.y + p.h > platTop - (GRID_CELL_SIZE / 6.0f) && p.velY <= 0) {
            // Player is falling and above platform surface
            return true;
        }
    }

    return false;
}
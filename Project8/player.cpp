#include "player.h"
#include "constants.h"
#include "obstacle.h"
#include <cmath>
#include <limits>

namespace {
constexpr float kSolveEps = 1e-4f;
constexpr float kOverlapEps = 1e-3f;
constexpr float kMinLandingTime = 0.02f;

bool solveDescendingHitTime(float startY, float startVelY, float targetY, float& hitTime) {
    // startY + startVelY*t + 0.5*GRAVITY*t^2 = targetY
    const float a = 0.5f * GRAVITY;
    const float b = startVelY;
    const float c = startY - targetY;
    const float disc = b * b - 4.0f * a * c;

    if (disc < 0.0f) {
        return false;
    }

    const float sqrtDisc = std::sqrt(disc);
    const float denom = 2.0f * a;
    const float roots[2] = {
        (-b - sqrtDisc) / denom,
        (-b + sqrtDisc) / denom
    };

    hitTime = std::numeric_limits<float>::infinity();
    for (float t : roots) {
        if (t <= kSolveEps) {
            continue;
        }
        const float velAtHit = startVelY + GRAVITY * t;
        if (velAtHit > 0.0f) {
            continue;
        }
        if (t < hitTime) {
            hitTime = t;
        }
    }

    return std::isfinite(hitTime);
}
}  // namespace

Player::Player() : worldX(0), y(GROUND_Y), velY(0), onGround(true), w(PLAYER_WIDTH), h(PLAYER_HEIGHT), rotation(0), airRotationSpeed(ROTATION_SPEED), timeInAir(0), prevY(GROUND_Y) {
}

void Player::reset() {
    worldX = 0;
    y = GROUND_Y;
    velY = 0;
    onGround = true;
    rotation = 0;
    airRotationSpeed = ROTATION_SPEED;
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
        airRotationSpeed = ROTATION_SPEED;
        // Snap rotation to nearest 90° when landing
        rotation = snapRotationTo90(rotation);
    } else if (velY != 0.0f) {
        onGround = false;
        // Rotate whenever falling or jumping (experiencing gravity/movement)
        timeInAir += dt;
        rotation += airRotationSpeed * dt;
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
        airRotationSpeed = ROTATION_SPEED;
        timeInAir = 0.0f;
    }
}

// Snap rotation to nearest 90° increment (0°, 90°, 180°, 270°)
float snapRotationTo90(float rotation) {
    return std::round(rotation / 90.0f) * 90.0f;
}

float computeAirRotationSpeed(const Player& p,
                              const std::vector<Obstacle>& obstacles,
                              float camX,
                              float groundY) {
    if (p.onGround) {
        return ROTATION_SPEED;
    }

    float timeToLanding;
    if (!solveDescendingHitTime(p.y, p.velY, groundY, timeToLanding)) {
        return ROTATION_SPEED;
    }

    const float playerLeft = p.screenX();
    const float playerRight = playerLeft + p.w;

    // Find the earliest reachable platform top by projecting platform position at impact time.
    for (const auto& obs : obstacles) {
        if (obs.type != ObstacleType::PLATFORM) {
            continue;
        }

        const float platformTop = obs.y + obs.h;
        float platformHitTime;
        if (!solveDescendingHitTime(p.y, p.velY, platformTop, platformHitTime)) {
            continue;
        }

        const float platformLeftNow = obs.worldX - camX;
        const float platformLeftAtHit = platformLeftNow - SCROLL_SPEED * platformHitTime;
        const float platformRightAtHit = platformLeftAtHit + obs.w;
        const bool horizontalOverlapAtHit =
            (playerRight > platformLeftAtHit - kOverlapEps) &&
            (playerLeft < platformRightAtHit + kOverlapEps);

        if (horizontalOverlapAtHit && platformHitTime < timeToLanding) {
            timeToLanding = platformHitTime;
        }
    }

    if (timeToLanding < kMinLandingTime) {
        return ROTATION_SPEED;
    }

    // Stay close to base spin rate, but guarantee a flat side at impact.
    const float baseDelta = ROTATION_SPEED * timeToLanding;
    const float desiredFinal = p.rotation + baseDelta;
    const float snappedFinal = snapRotationTo90(desiredFinal);
    const float rawDelta = snappedFinal - p.rotation;
    const float turnOffset = std::round((baseDelta - rawDelta) / 360.0f) * 360.0f;
    const float adjustedDelta = rawDelta + turnOffset;

    return adjustedDelta / timeToLanding;
}

float Player::screenX() const {
    return PLAYER_SCREEN_X;
}

#ifndef PLAYER_H
#define PLAYER_H

#include <vector>

struct Obstacle;

struct Player {
    float worldX;       // tracks distance traveled (for win check)
    float y;            // screen Y position (changes with physics)
    float velY;         // vertical velocity
    bool onGround;      // whether player is on the ground
    float w, h;         // dimensions
    float rotation;     // current rotation angle in degrees
    float airRotationSpeed; // adaptive rotation speed while airborne
    float timeInAir;    // time spent in air since jump
    float prevY;        // previous y position (for collision checks)

    Player();
    void reset();
    void update(float dt, float groundY);
    void jump();

    // Utility: returns screen X position (always fixed)
    float screenX() const;
};

// Snap rotation to nearest 90° increment (0°, 90°, 180°, 270°)
float snapRotationTo90(float rotation);

// Predict next landing surface and compute a speed that lands flat.
float computeAirRotationSpeed(const Player& p,
                              const std::vector<Obstacle>& obstacles,
                              float camX,
                              float groundY);

#endif

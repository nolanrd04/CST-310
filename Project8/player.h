#ifndef PLAYER_H
#define PLAYER_H

struct Player {
    float worldX;       // tracks distance traveled (for win check)
    float y;            // screen Y position (changes with physics)
    float velY;         // vertical velocity
    bool onGround;      // whether player is on the ground
    float w, h;         // dimensions

    Player();
    void reset();
    void update(float dt, float groundY);
    void jump();

    // Utility: returns screen X position (always fixed)
    float screenX() const;
};

#endif
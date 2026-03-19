#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <glm/glm.hpp>

enum class ObstacleType { SPIKE, PLATFORM };

struct Obstacle {
    ObstacleType type;
    float worldX, y;
    float w, h;
    glm::vec3 color;

    Obstacle(ObstacleType t, float x, float posY, float width, float height, glm::vec3 col);
};

// Forward declaration
struct Player;

// Collision detection
bool checkCollisionWithSpike(const Player& p, const Obstacle& spike, float camX);
bool checkCollisionWithPlatform(const Player& p, const Obstacle& platform, float camX);

#endif
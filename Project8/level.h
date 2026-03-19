#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include "obstacle.h"

struct Level {
    std::vector<Obstacle> obstacles;
    float length;       // total level length
    float cameraX;      // current scroll position
    int seed;

    Level();
    void generate(int levelSeed);
    void update(float dt);
    bool isFinished(float playerWorldX) const;
    float getFinishLineX() const;
};

#endif
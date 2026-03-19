#include "level.h"
#include "constants.h"
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>

Level::Level() : length(LEVEL_LENGTH), cameraX(0), seed(42) {
}

void Level::generate(int levelSeed) {
    obstacles.clear();
    seed = levelSeed;
    srand(seed);
    cameraX = 0;

    float x = 600.0f;  // start position (give runway)

    while (x < LEVEL_LENGTH - 600.0f) {
        // 60% spike, 40% platform
        int roll = rand() % 100;

        if (roll < 60) {
            // Spike cluster: 1-2 consecutive spikes (max 2 to keep level playable)
            int numSpikes = 1 + (rand() % 2);
            for (int i = 0; i < numSpikes; i++) {
                Obstacle spike(ObstacleType::SPIKE, x, GROUND_Y, 50.0f, 80.0f,
                               glm::vec3(1.0f, 1.0f, 1.0f));  // white spikes
                obstacles.push_back(spike);
                x += 50.0f + 20.0f;  // spike width + gap
            }
            x -= 20.0f;  // remove last gap
        } else {
            // Platform: elevated, variable width
            float platformWidth = 150.0f + (rand() % 150);
            float platformHeight = 20.0f;
            float platformY = GROUND_Y + 80.0f + (rand() % 100);  // 80-180 units above ground

            Obstacle platform(ObstacleType::PLATFORM, x, platformY, platformWidth, platformHeight,
                             glm::vec3(PLATFORM_R, PLATFORM_G, PLATFORM_B));
            obstacles.push_back(platform);
        }

        // Gap to next obstacle: 250-450 units
        x += 250.0f + (rand() % 200);
    }

    // Add finish line (gold platform at end)
    float finishX = LEVEL_LENGTH - 200.0f;
    Obstacle finishLine(ObstacleType::PLATFORM, finishX, GROUND_Y, 100.0f, 150.0f,
                       glm::vec3(FINISH_R, FINISH_G, FINISH_B));
    obstacles.push_back(finishLine);
}

void Level::update(float dt) {
    cameraX += SCROLL_SPEED * dt;
}

bool Level::isFinished(float playerWorldX) const {
    return playerWorldX >= LEVEL_LENGTH;
}

float Level::getFinishLineX() const {
    return LEVEL_LENGTH - 200.0f;
}
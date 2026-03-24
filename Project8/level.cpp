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

    float x = RUNWAY_CELLS * GRID_CELL_SIZE;  // start position (give runway)

    while (x < LEVEL_LENGTH - BUFFER_CELLS * GRID_CELL_SIZE) {
        // 60% spike, 40% platform
        int roll = rand() % 100;

        if (roll < 60) {
            // Spike cluster: 1-2 consecutive spikes (max 2 to keep level playable)
            int numSpikes = 1 + (rand() % 2);
            for (int i = 0; i < numSpikes; i++) {
                Obstacle spike(ObstacleType::SPIKE, x, GROUND_Y, SPIKE_W, SPIKE_H,
                               glm::vec3(1.0f, 1.0f, 1.0f));  // white spikes
                obstacles.push_back(spike);
                x += SPIKE_W + SPIKE_INTRA_GAP;  // spike width + gap
            }
            x -= SPIKE_INTRA_GAP;  // remove last gap
        } else {
            // Platform: elevated, variable width
            float platformWidth  = (PLATFORM_MIN_CELLS + (rand() % (int)PLATFORM_RANGE_CELLS)) * GRID_CELL_SIZE;
            float platformHeight = PLATFORM_H;
            float platformY      = GROUND_Y + (PLAT_ELEV_MIN_CELLS + (rand() % (int)PLAT_ELEV_RANGE_CELLS)) * GRID_CELL_SIZE;

            Obstacle platform(ObstacleType::PLATFORM, x, platformY, platformWidth, platformHeight,
                             glm::vec3(PLATFORM_R, PLATFORM_G, PLATFORM_B));
            obstacles.push_back(platform);
        }

        // Gap to next obstacle: 4-7 cells
        x += (GAP_MIN_CELLS + (rand() % (int)GAP_RANGE_CELLS)) * GRID_CELL_SIZE;
    }

    // Add finish line (gold platform at end)
    float finishX = LEVEL_LENGTH - 3.0f * GRID_CELL_SIZE;
    Obstacle finishLine(ObstacleType::PLATFORM, finishX, GROUND_Y, FINISH_W, FINISH_H,
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
    return LEVEL_LENGTH - 3.0f * GRID_CELL_SIZE;
}
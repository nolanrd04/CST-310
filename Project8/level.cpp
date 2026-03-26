#include "level.h"
#include "constants.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <unordered_set>
#include <glm/glm.hpp>

namespace {
enum class PlatformHazard { NONE, TOP_IMPASSABLE, UNDER_IMPASSABLE };
enum class SafeLane { GROUND, TOP };

struct PlatformPlan {
    float x;
    float width;
    float topY;
    PlatformHazard hazard;
    SafeLane laneAfter;
};

float computeMaxJumpHeight() {
    return (JUMP_VEL * JUMP_VEL) / (-2.0f * GRAVITY);
}

bool solveDescendingJumpTimeToDeltaY(float deltaY, float& hitTime) {
    // JUMP_VEL*t + 0.5*GRAVITY*t^2 = deltaY
    const float a = 0.5f * GRAVITY;
    const float b = JUMP_VEL;
    const float c = -deltaY;
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

    float best = -1.0f;
    for (float t : roots) {
        if (t <= 0.0f) {
            continue;
        }
        const float velAtT = JUMP_VEL + GRAVITY * t;
        if (velAtT > 0.0f) {
            continue;
        }
        if (best < 0.0f || t < best) {
            best = t;
        }
    }

    if (best <= 0.0f) {
        return false;
    }

    hitTime = best;
    return true;
}

float computeJumpRangeToTargetY(float fromY, float toY) {
    float t = 0.0f;
    if (!solveDescendingJumpTimeToDeltaY(toY - fromY, t)) {
        return -1.0f;
    }
    return SCROLL_SPEED * t;
}

bool canReachTopLane(float fromY, float toY, float requiredRange) {
    const float range = computeJumpRangeToTargetY(fromY, toY);
    return range >= requiredRange;
}

int chooseNextPlatformTier(int currentTier) {
    // Keep elevation changes bounded to one tier so every jump remains reachable.
    const int roll = rand() % 100;
    if (currentTier == 1) {
        return (roll < 35) ? 2 : 1;
    }
    return (roll < 45) ? 1 : 2;
}

int randomInRangeInclusive(int minValue, int maxValue) {
    if (maxValue <= minValue) {
        return minValue;
    }
    return minValue + (rand() % (maxValue - minValue + 1));
}

struct SimState {
    float y;
    float velY;
    bool onGround;
};

struct StateKey {
    int yBin;
    int vBin;
    bool onGround;

    bool operator==(const StateKey& other) const {
        return yBin == other.yBin &&
               vBin == other.vBin &&
               onGround == other.onGround;
    }
};

struct StateKeyHasher {
    std::size_t operator()(const StateKey& key) const {
        std::size_t h = 1469598103934665603ull;
        h ^= static_cast<std::size_t>(key.yBin + 100000);
        h *= 1099511628211ull;
        h ^= static_cast<std::size_t>(key.vBin + 100000);
        h *= 1099511628211ull;
        h ^= static_cast<std::size_t>(key.onGround ? 1 : 0);
        h *= 1099511628211ull;
        return h;
    }
};

StateKey quantizeState(const SimState& s) {
    StateKey key;
    key.yBin = static_cast<int>(std::lround(s.y / 3.0f));
    key.vBin = static_cast<int>(std::lround(s.velY / 25.0f));
    key.onGround = s.onGround;
    return key;
}

bool aabbOverlap(float ax1, float ay1, float aw, float ah,
                 float bx1, float by1, float bw, float bh) {
    return ax1 < bx1 + bw &&
           ax1 + aw > bx1 &&
           ay1 < by1 + bh &&
           ay1 + ah > by1;
}

bool hitsSpike(float playerY, const Obstacle& spike, float camX) {
    const float spikeScreenX = spike.worldX - camX;
    const float playerScreenX = PLAYER_SCREEN_X;
    const float sqSize = spike.w * 0.55f;
    const float sqX = spikeScreenX + (spike.w - sqSize) / 2.0f;
    const float sqY = spike.y;

    return aabbOverlap(playerScreenX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT,
                       sqX, sqY, sqSize, sqSize);
}

bool hitsPlatformTop(float playerY, float playerVelY, const Obstacle& platform, float camX) {
    const float platScreenX = platform.worldX - camX;
    const float playerScreenX = PLAYER_SCREEN_X;
    const float platTop = platform.y + platform.h;

    if (playerScreenX + PLAYER_WIDTH <= platScreenX ||
        playerScreenX >= platScreenX + platform.w) {
        return false;
    }

    return playerY <= platTop &&
           playerY + PLAYER_HEIGHT > platTop - (GRID_CELL_SIZE / 6.0f) &&
           playerVelY <= 0.0f;
}

bool isLayoutPlayable(const std::vector<Obstacle>& layout) {
    const float dx = SCROLL_SPEED * DT;
    const int maxFrames = static_cast<int>(LEVEL_LENGTH / dx) + 240;
    std::vector<SimState> frontier;
    frontier.push_back({GROUND_Y, 0.0f, true});

    for (int frame = 0; frame < maxFrames; frame++) {
        const float camXNext = (frame + 1) * dx;
        std::vector<SimState> nextFrontier;
        nextFrontier.reserve(frontier.size() * 2);
        std::unordered_set<StateKey, StateKeyHasher> dedupe;

        for (const auto& state : frontier) {
            const int actionCount = state.onGround ? 2 : 1;
            for (int action = 0; action < actionCount; action++) {
                SimState sim = state;
                if (action == 1 && sim.onGround) {
                    sim.velY = JUMP_VEL;
                    sim.onGround = false;
                }

                const float prevY = sim.y;
                sim.velY += GRAVITY * DT;
                sim.y += sim.velY * DT;

                if (sim.y <= GROUND_Y) {
                    sim.y = GROUND_Y;
                    sim.velY = 0.0f;
                    sim.onGround = true;
                }

                bool dead = false;
                for (const auto& obs : layout) {
                    if (obs.type == ObstacleType::SPIKE) {
                        if (hitsSpike(sim.y, obs, camXNext)) {
                            dead = true;
                            break;
                        }
                    } else if (obs.type == ObstacleType::PLATFORM) {
                        if (hitsPlatformTop(sim.y, sim.velY, obs, camXNext)) {
                            const float platTop = obs.y + obs.h;
                            if (prevY >= platTop) {
                                sim.y = platTop;
                                sim.velY = 0.0f;
                                sim.onGround = true;
                            }
                        }
                    }
                }

                if (dead) {
                    continue;
                }
                if (camXNext >= LEVEL_LENGTH) {
                    return true;
                }

                const StateKey key = quantizeState(sim);
                if (dedupe.insert(key).second) {
                    nextFrontier.push_back(sim);
                }
            }
        }

        frontier.swap(nextFrontier);
        if (frontier.empty()) {
            return false;
        }
    }

    return false;
}
}  // namespace

Level::Level() : length(LEVEL_LENGTH), cameraX(0), seed(42) {
}

void Level::generate(int levelSeed) {
    seed = levelSeed;
    cameraX = 0;

    // Platform layout with random widths.
    const float gapBetweenPlatformsCells = 3.0f;  // fixed gap between end of one and start of next
    const float gapBetweenPlatforms = gapBetweenPlatformsCells * GRID_CELL_SIZE;
    const float transitionRangeRequirement = gapBetweenPlatforms + (0.25f * GRID_CELL_SIZE);

    // Player ability model.
    const float maxJumpHeight = computeMaxJumpHeight();
    const float safeJumpHeight = maxJumpHeight * 0.85f;
    const float tier1Top = GROUND_Y + safeJumpHeight;
    const float tier2Top = GROUND_Y + (2.0f * safeJumpHeight);
    const float maxGroundJumpRange = computeJumpRangeToTargetY(GROUND_Y, GROUND_Y);
    const float maxGroundHazardWidth = maxGroundJumpRange - GRID_CELL_SIZE;

    // Hazard ranges.
    const int minGapSpikeGroup = 1;
    const int maxGapSpikeGroup = 2;
    const int minImpassablePlatformSpikes = 3;
    const float finishX = LEVEL_LENGTH - 3.0f * GRID_CELL_SIZE;
    const float generationEnd = finishX - BUFFER_CELLS * GRID_CELL_SIZE;
    const int maxGenerationAttempts = 24;

    auto buildCandidate = [&](int gapSpikeGroupChance,
                              int platformTopSpikeChance,
                              int platformUnderSpikeChance,
                              int topSpikeWhileOnTopChance) {
        obstacles.clear();

        // Build platform anchors with random widths.
        std::vector<PlatformPlan> plans;
        float x = RUNWAY_CELLS * GRID_CELL_SIZE;  // start position (give runway)
        int tier = 1;  // first elevated platform is always tier 1 (reachable from ground).
        while (true) {
            // Random width for this platform
            int widthCells = randomInRangeInclusive(
                static_cast<int>(PLATFORM_MIN_CELLS),
                static_cast<int>(PLATFORM_MIN_CELLS + PLATFORM_RANGE_CELLS));
            float thisWidth = widthCells * GRID_CELL_SIZE;

            if (x + thisWidth >= generationEnd) break;

            if (!plans.empty()) {
                tier = chooseNextPlatformTier(tier);
            }

            PlatformPlan plan;
            plan.x = x;
            plan.width = thisWidth;
            plan.topY = (tier == 1) ? tier1Top : tier2Top;
            plan.hazard = PlatformHazard::NONE;
            plan.laneAfter = SafeLane::GROUND;
            plans.push_back(plan);

            x += thisWidth + gapBetweenPlatforms;
        }

        // Ability-aware hazard planning with a guaranteed path lane.
        SafeLane safeLane = SafeLane::GROUND;
        float safeLaneY = GROUND_Y;

        for (size_t i = 0; i < plans.size(); i++) {
            auto& plan = plans[i];
            const float platformY = plan.topY - PLATFORM_H;
            const bool canFitUnderPlatform = (GROUND_Y + SPIKE_H) < platformY;
            const bool canReachTopFromSafe = canReachTopLane(safeLaneY, plan.topY, transitionRangeRequirement);
            const int hazardRoll = rand() % 100;

            // Keep the first platform clear so the initial transition is never unfair.
            if (i == 0) {
                plan.hazard = PlatformHazard::NONE;
                safeLane = SafeLane::GROUND;
                safeLaneY = GROUND_Y;
                plan.laneAfter = safeLane;
                continue;
            }

            PlatformHazard chosen = PlatformHazard::NONE;

            if (safeLane == SafeLane::GROUND) {
                // Only block ground if player can reliably transition up.
                if (ENABLE_PLATFORM_UNDER_SPIKES &&
                    canFitUnderPlatform &&
                    canReachTopFromSafe &&
                    hazardRoll < platformUnderSpikeChance) {
                    chosen = PlatformHazard::UNDER_IMPASSABLE;
                    safeLane = SafeLane::TOP;
                    safeLaneY = plan.topY;
                } else if (ENABLE_PLATFORM_TOP_SPIKES &&
                           hazardRoll < platformUnderSpikeChance + platformTopSpikeChance) {
                    chosen = PlatformHazard::TOP_IMPASSABLE;
                    safeLane = SafeLane::GROUND;
                    safeLaneY = GROUND_Y;
                } else {
                    chosen = PlatformHazard::NONE;
                    safeLane = SafeLane::GROUND;
                    safeLaneY = GROUND_Y;
                }
            } else {
                // Currently safe on top lane.
                if (!canReachTopFromSafe) {
                    // Top-to-top transition is not reachable, so naturally return to ground lane.
                    chosen = PlatformHazard::NONE;
                    safeLane = SafeLane::GROUND;
                    safeLaneY = GROUND_Y;
                } else if (ENABLE_PLATFORM_UNDER_SPIKES &&
                           canFitUnderPlatform &&
                           hazardRoll < platformUnderSpikeChance) {
                    chosen = PlatformHazard::UNDER_IMPASSABLE;
                    safeLane = SafeLane::TOP;
                    safeLaneY = plan.topY;
                } else if (ENABLE_PLATFORM_TOP_SPIKES &&
                           hazardRoll < platformUnderSpikeChance + topSpikeWhileOnTopChance) {
                    chosen = PlatformHazard::TOP_IMPASSABLE;
                    safeLane = SafeLane::GROUND;
                    safeLaneY = GROUND_Y;
                } else {
                    chosen = PlatformHazard::NONE;
                    safeLane = SafeLane::TOP;
                    safeLaneY = plan.topY;
                }
            }

            // Keep the final generated platform before finish more forgiving.
            if (i + 1 == plans.size() && chosen == PlatformHazard::TOP_IMPASSABLE) {
                chosen = PlatformHazard::NONE;
            }

            plan.hazard = chosen;
            plan.laneAfter = safeLane;
        }

        // Emit platforms + platform hazards.
        for (const auto& plan : plans) {
            const float platformY = plan.topY - PLATFORM_H;
            Obstacle platform(ObstacleType::PLATFORM, plan.x, platformY, plan.width, PLATFORM_H,
                              glm::vec3(PLATFORM_R, PLATFORM_G, PLATFORM_B));
            obstacles.push_back(platform);

            const int maxPlatformSpikeCount = static_cast<int>(plan.width / SPIKE_W);
            const int minPlatformSpikeCount = std::min(minImpassablePlatformSpikes, maxPlatformSpikeCount);

            if (plan.hazard == PlatformHazard::NONE || maxPlatformSpikeCount <= 0 || minPlatformSpikeCount <= 0) {
                continue;
            }

            const int platformSpikeCount = randomInRangeInclusive(minPlatformSpikeCount, maxPlatformSpikeCount);
            const float clusterWidth = platformSpikeCount * SPIKE_W;
            const float clusterStartX = plan.x + (plan.width - clusterWidth) / 2.0f;

            for (int i = 0; i < platformSpikeCount; i++) {
                const float spikeX = clusterStartX + i * SPIKE_W;
                const float spikeY = (plan.hazard == PlatformHazard::TOP_IMPASSABLE) ? plan.topY : GROUND_Y;
                Obstacle spike(ObstacleType::SPIKE, spikeX, spikeY, SPIKE_W, SPIKE_H,
                               glm::vec3(1.0f, 1.0f, 1.0f));
                obstacles.push_back(spike);
            }
        }

        // Emit random 1-2 spike groups in gaps, gated by the guaranteed safe lane.
        for (size_t i = 0; i + 1 < plans.size(); i++) {
            if ((rand() % 100) >= gapSpikeGroupChance) {
                continue;
            }

            const bool safeGroundAfterThisPlatform = plans[i].laneAfter == SafeLane::GROUND;
            const bool nextPlatformBlocksGround = plans[i + 1].hazard == PlatformHazard::UNDER_IMPASSABLE;
            if (safeGroundAfterThisPlatform && nextPlatformBlocksGround) {
                // Keep the required lane-switch gap clean.
                continue;
            }

            // Avoid forcing a jump directly under/into top-impassable platforms.
            const bool aroundTopImpassable =
                plans[i].hazard == PlatformHazard::TOP_IMPASSABLE ||
                plans[i + 1].hazard == PlatformHazard::TOP_IMPASSABLE;
            if (aroundTopImpassable) {
                continue;
            }

            const float gapStart = plans[i].x + plans[i].width;
            const float gapEnd = plans[i + 1].x;
            const float gapWidth = gapEnd - gapStart;
            const int maxGapSpikeCountBySpace = static_cast<int>(gapWidth / SPIKE_W);
            const int groupMax = std::min(maxGapSpikeGroup, maxGapSpikeCountBySpace);
            if (groupMax < minGapSpikeGroup) {
                continue;
            }

            const int spikeCount = randomInRangeInclusive(minGapSpikeGroup, groupMax);
            const float groupWidth = spikeCount * SPIKE_W;
            if (groupWidth > maxGroundHazardWidth) {
                continue;
            }

            float groupStartX = gapStart;
            const float xSlack = gapWidth - groupWidth;
            if (xSlack > 0.0f) {
                const float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                groupStartX += rand01 * xSlack;
            }

            for (int s = 0; s < spikeCount; s++) {
                const float spikeX = groupStartX + (s * SPIKE_W);
                Obstacle gapSpike(ObstacleType::SPIKE, spikeX, GROUND_Y, SPIKE_W, SPIKE_H,
                                  glm::vec3(1.0f, 1.0f, 1.0f));
                obstacles.push_back(gapSpike);
            }
        }

        // Add finish line (gold platform at end)
        Obstacle finishLine(ObstacleType::PLATFORM, finishX, GROUND_Y, FINISH_W, FINISH_H,
                            glm::vec3(FINISH_R, FINISH_G, FINISH_B));
        obstacles.push_back(finishLine);
    };

    bool playable = false;
    int chosenSeed = levelSeed;
    for (int attempt = 0; attempt < maxGenerationAttempts; attempt++) {
        const int attemptSeed = levelSeed + attempt * 7919;
        srand(attemptSeed);

        const bool relaxed = attempt >= (maxGenerationAttempts / 2);
        const int gapSpikeGroupChance = relaxed ? 25 : 35;
        const int platformTopSpikeChance = relaxed ? 12 : 18;
        const int platformUnderSpikeChance = relaxed ? 12 : 18;
        const int topSpikeWhileOnTopChance = relaxed ? 6 : 10;

        buildCandidate(gapSpikeGroupChance,
                       platformTopSpikeChance,
                       platformUnderSpikeChance,
                       topSpikeWhileOnTopChance);

        if (isLayoutPlayable(obstacles)) {
            playable = true;
            chosenSeed = attemptSeed;
            break;
        }
    }

    if (!playable) {
        // Guaranteed fallback: keep platform structure but remove all spikes.
        buildCandidate(0, 0, 0, 0);
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                           [](const Obstacle& obs) { return obs.type == ObstacleType::SPIKE; }),
            obstacles.end());
    }

    seed = chosenSeed;
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

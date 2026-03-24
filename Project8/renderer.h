#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <vector>

// Forward declaration
struct Obstacle;
struct Player;

// Renderer initialization
void initRenderer();

// Debug visualization
extern bool DRAW_HITBOXES;   // Toggle hitbox rendering with 'H'
extern bool SHOW_FPS;        // Toggle FPS counter with 'F'
void drawHitboxes(const Player& player, const std::vector<Obstacle>& obs, float camX);
void drawFpsCounter();

// Time tracking for animations
void updateAnimationTime(float dt);
void updateFpsCounter();

// Color animation
glm::vec3 getAnimatedSkyColor();
glm::vec3 getAnimatedGroundColor();

// Draw functions
void drawQuad(float screenX, float screenY, float w, float h, glm::vec3 color);
void drawTriangle(float screenX, float screenY, float w, float h, glm::vec3 color);
void drawTriangleWithOutline(float screenX, float screenY, float w, float h, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineWidth);
void drawBackground();
void drawGround(float camX);
void drawObstacles(const std::vector<Obstacle>& obs, float camX);
void drawHUD(int gameState);

#endif
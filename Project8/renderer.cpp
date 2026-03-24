#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include "renderer.h"
#include "constants.h"
#include "obstacle.h"
#include "player.h"

static float animationTime = 0.0f;
bool DRAW_HITBOXES = false;  // Toggle with 'H' key
bool SHOW_FPS = false;        // Toggle with 'F' key

// FPS counter
static int frameCount = 0;
static int lastFpsTime = 0;
static int currentFps = 0;

void updateAnimationTime(float dt) {
    animationTime += dt;
    if (animationTime > COLOR_CYCLE_SPEED) {
        animationTime -= COLOR_CYCLE_SPEED;  // loop the animation
    }
}

void updateFpsCounter() {
    frameCount++;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    if (currentTime - lastFpsTime >= 1000) {  // Update every second
        currentFps = frameCount;
        frameCount = 0;
        lastFpsTime = currentTime;
    }
}

// Convert hue (0-1) to RGB with specified brightness
static glm::vec3 hueToRgb(float hue, float brightness) {
    // Hue rotation through green -> cyan -> blue -> magenta -> red -> yellow -> green
    float h = hue * 6.0f;  // 0-6 for 6 color zones
    float saturation = 0.6f;  // constant saturation
    float c = saturation * brightness;
    float x = c * (1.0f - std::fabs(std::fmod(h, 2.0f) - 1.0f));

    glm::vec3 rgb;
    if (h < 1.0f) rgb = glm::vec3(c, x, 0.0f);           // red -> yellow
    else if (h < 2.0f) rgb = glm::vec3(x, c, 0.0f);      // yellow -> green
    else if (h < 3.0f) rgb = glm::vec3(0.0f, c, x);      // green -> cyan
    else if (h < 4.0f) rgb = glm::vec3(0.0f, x, c);      // cyan -> blue
    else if (h < 5.0f) rgb = glm::vec3(x, 0.0f, c);      // blue -> magenta
    else rgb = glm::vec3(c, 0.0f, x);                     // magenta -> red

    // Add constant value for brightness
    float m = brightness - c / 2.0f;
    return rgb + glm::vec3(m, m, m);
}

glm::vec3 getAnimatedSkyColor() {
    // Dark background (low brightness)
    float hue = std::fmod(animationTime / COLOR_CYCLE_SPEED, 1.0f);
    return hueToRgb(hue, 0.25f);  // dark background
}

glm::vec3 getAnimatedGroundColor() {
    // Same hue as background but lighter (bright ground)
    float hue = std::fmod(animationTime / COLOR_CYCLE_SPEED, 1.0f);
    return hueToRgb(hue, 0.50f);  // lighter ground with same hue
}

void initRenderer() {
    glClearColor(0.2f, 0.3f, 0.2f, 1.0f);  // neutral green base color
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, WINDOW_W, WINDOW_H);

    // Setup 2D projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WINDOW_W, 0.0, WINDOW_H, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawQuad(float x, float y, float w, float h, glm::vec3 color) {
    glColor3f(color.x, color.y, color.z);
    glRectf(x, y, x + w, y + h);
}

void drawQuadRotated(float x, float y, float w, float h, float rotation, glm::vec3 color) {
    glColor3f(color.x, color.y, color.z);

    // Save the current matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Translate to the center of the quad
    float centerX = x + w / 2.0f;
    float centerY = y + h / 2.0f;
    glTranslatef(centerX, centerY, 0.0f);

    // Rotate around Z axis
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);

    // Draw quad centered at origin
    glRectf(-w / 2.0f, -h / 2.0f, w / 2.0f, h / 2.0f);

    // Restore the matrix
    glPopMatrix();
}

void drawQuadRotatedWithOutline(float x, float y, float w, float h, float rotation, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineWidth) {
    // Save the current matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Translate to the center of the quad
    float centerX = x + w / 2.0f;
    float centerY = y + h / 2.0f;
    glTranslatef(centerX, centerY, 0.0f);

    // Rotate around Z axis
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);

    // Draw larger outline first
    float outlineScale = 1.0f + (outlineWidth / w);
    float outlineW = w * outlineScale;
    float outlineH = h * outlineScale;

    glColor3f(outlineColor.x, outlineColor.y, outlineColor.z);
    glRectf(-outlineW / 2.0f, -outlineH / 2.0f, outlineW / 2.0f, outlineH / 2.0f);

    // Draw filled quad on top
    glColor3f(fillColor.x, fillColor.y, fillColor.z);
    glRectf(-w / 2.0f, -h / 2.0f, w / 2.0f, h / 2.0f);

    // Restore the matrix
    glPopMatrix();
}

void drawPlayerFace(float x, float y, float w, float h, float rotation) {
    // Draw eyes and mouth on the player, rotated with it
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Translate to center and rotate
    float centerX = x + w / 2.0f;
    float centerY = y + h / 2.0f;
    glTranslatef(centerX, centerY, 0.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);

    glColor3f(0.0f, 0.0f, 1.0f);  // blue eyes and mouth

    // Eye dimensions
    float eyeSize = w * 0.2f;      // 20% of player width
    float eyeSpacing = w * 0.25f;  // eyes positioned 25% from center
    float eyeYOffset = -h * 0.15f; // eyes positioned slightly above center

    // Left eye
    glRectf(-eyeSpacing - eyeSize / 2.0f, eyeYOffset - eyeSize / 2.0f,
            -eyeSpacing + eyeSize / 2.0f, eyeYOffset + eyeSize / 2.0f);

    // Right eye
    glRectf(eyeSpacing - eyeSize / 2.0f, eyeYOffset - eyeSize / 2.0f,
            eyeSpacing + eyeSize / 2.0f, eyeYOffset + eyeSize / 2.0f);

    // Mouth - thin rectangle spanning between eyes
    float mouthWidth = eyeSpacing * 2.0f + eyeSize;  // from left edge of left eye to right edge of right eye
    float mouthHeight = h * 0.08f;                    // thin rectangle
    float mouthYOffset = h * 0.05f;                   // below the eyes

    glRectf(-mouthWidth / 2.0f, mouthYOffset - mouthHeight / 2.0f,
            mouthWidth / 2.0f, mouthYOffset + mouthHeight / 2.0f);

    glPopMatrix();
}

void drawPlatformWithGradient(float x, float y, float w, float h, glm::vec3 outlineColor, float outlineWidth) {
    // Draw white outline border as thick lines
    glColor3f(outlineColor.x, outlineColor.y, outlineColor.z);
    glLineWidth(outlineWidth);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f);  // Reset line width

    // Draw gradient quad (top = opaque dark, bottom = transparent)
    glBegin(GL_QUADS);

    // Top-left: opaque dark gray
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glVertex2f(x, y + h);

    // Top-right: opaque dark gray
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glVertex2f(x + w, y + h);

    // Bottom-right: transparent
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
    glVertex2f(x + w, y);

    // Bottom-left: transparent
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
    glVertex2f(x, y);

    glEnd();
}

void drawTriangle(float x, float y, float w, float h, glm::vec3 color) {
    // Draw equilateral triangle: height = width * sqrt(3)/2
    float eqHeight = w * std::sqrt(3.0f) / 2.0f;

    glColor3f(color.x, color.y, color.z);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);              // bottom-left
    glVertex2f(x + w, y);          // bottom-right
    glVertex2f(x + w/2.0f, y + eqHeight); // top-center
    glEnd();
}

void drawTriangleWithOutline(float x, float y, float w, float h, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineWidth) {
    // Draw equilateral triangles with outline
    float eqHeight = w * std::sqrt(3.0f) / 2.0f;

    // Draw larger outline first
    float outlineScale = 1.0f + (outlineWidth / w);
    float offsetX = w * (outlineScale - 1.0f) / 2.0f;
    float offsetY = eqHeight * (outlineScale - 1.0f);

    glColor3f(outlineColor.x, outlineColor.y, outlineColor.z);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - offsetX, y);                              // bottom-left
    glVertex2f(x + w + offsetX, y);                          // bottom-right
    glVertex2f(x + w/2.0f, y + eqHeight + offsetY);         // top-center
    glEnd();

    // Draw black fill on top
    glColor3f(fillColor.x, fillColor.y, fillColor.z);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);              // bottom-left
    glVertex2f(x + w, y);          // bottom-right
    glVertex2f(x + w/2.0f, y + eqHeight); // top-center
    glEnd();
}

void drawBackground() {
    glm::vec3 skyColor = getAnimatedSkyColor();
    glColor3f(skyColor.x, skyColor.y, skyColor.z);
    glRectf(0.0f, 0.0f, WINDOW_W, WINDOW_H);
}

void drawGround(float camX) {
    // Top green strip with animation
    glm::vec3 groundColor = getAnimatedGroundColor();
    glColor3f(groundColor.x, groundColor.y, groundColor.z);
    glRectf(0.0f, 0.0f, WINDOW_W, GROUND_Y);

    // Bottom dark strip
    glColor3f(GROUND_BOT_R, GROUND_BOT_G, GROUND_BOT_B);
    glRectf(0.0f, -50.0f, WINDOW_W, 0.0f);
}

void drawObstacles(const std::vector<Obstacle>& obs, float camX) {
    for (const auto& obstacle : obs) {
        float screenX = obstacle.worldX - camX;

        // Skip if off-screen
        if (screenX + obstacle.w < 0 || screenX > WINDOW_W) {
            continue;
        }

        if (obstacle.type == ObstacleType::SPIKE) {
            // Draw spike with black fill and white outline
            drawTriangleWithOutline(screenX, obstacle.y, obstacle.w, obstacle.h,
                                   glm::vec3(SPIKE_FILL_R, SPIKE_FILL_G, SPIKE_FILL_B),
                                   glm::vec3(SPIKE_OUTLINE_R, SPIKE_OUTLINE_G, SPIKE_OUTLINE_B),
                                   3.0f);  // 3 pixel outline
        } else {
            // Draw platform with gradient fade and white outline
            drawPlatformWithGradient(screenX, obstacle.y, obstacle.w, obstacle.h,
                                    glm::vec3(1.0f, 1.0f, 1.0f),  // white outline
                                    3.0f);  // 3 pixel outline
        }
    }
}

static void renderString(float x, float y, void* font, const char* str) {
    glRasterPos2f(x, y);
    while (*str) {
        glutBitmapCharacter(font, *str);
        str++;
    }
}

void drawHUD(int gameState) {
    // Only render HUD in MENU, WIN, and DEAD states
    if (gameState != 0 && gameState != 2 && gameState != 3) {
        return;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_W, 0, WINDOW_H, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f); // white text

    // Display state message
    if (gameState == 0) { // MENU
        renderString(WINDOW_W / 2 - 200, WINDOW_H / 2 + 20, GLUT_BITMAP_TIMES_ROMAN_24, "PRESS SPACE TO START");
    } else if (gameState == 2) { // WIN
        renderString(WINDOW_W / 2 - 60, WINDOW_H / 2 + 20, GLUT_BITMAP_TIMES_ROMAN_24, "YOU WIN!");
        renderString(WINDOW_W / 2 - 120, WINDOW_H / 2 - 40, GLUT_BITMAP_HELVETICA_12, "Press R to restart");
    } else if (gameState == 3) { // DEAD
        renderString(WINDOW_W / 2 - 100, WINDOW_H / 2 + 20, GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER");
        renderString(WINDOW_W / 2 - 120, WINDOW_H / 2 - 40, GLUT_BITMAP_HELVETICA_12, "Press R to restart");
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void drawHitboxes(const Player& player, const std::vector<Obstacle>& obs, float camX) {
    if (!DRAW_HITBOXES) return;

    glDisable(GL_BLEND);  // Disable alpha blending for cleaner lines

    // Draw player hitbox (cyan) - rotated
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    float centerX = player.screenX() + player.w / 2.0f;
    float centerY = player.y + player.h / 2.0f;
    glTranslatef(centerX, centerY, 0.0f);
    glRotatef(player.rotation, 0.0f, 0.0f, 1.0f);

    glColor3f(0.0f, 1.0f, 1.0f);  // cyan
    glBegin(GL_LINE_LOOP);
    glVertex2f(-player.w / 2.0f, -player.h / 2.0f);
    glVertex2f(player.w / 2.0f, -player.h / 2.0f);
    glVertex2f(player.w / 2.0f, player.h / 2.0f);
    glVertex2f(-player.w / 2.0f, player.h / 2.0f);
    glEnd();

    glPopMatrix();

    // Draw obstacle hitboxes
    for (const auto& obstacle : obs) {
        float screenX = obstacle.worldX - camX;

        // Skip if off-screen
        if (screenX + obstacle.w < 0 || screenX > WINDOW_W) {
            continue;
        }

        if (obstacle.type == ObstacleType::SPIKE) {
            // Draw spike's actual square hitbox (magenta)
            float sqSize = obstacle.w * 0.65f;
            float sqX = screenX + (obstacle.w - sqSize) / 2.0f;
            float sqY = obstacle.y;

            glColor3f(1.0f, 0.0f, 1.0f);  // magenta
            glBegin(GL_LINE_LOOP);
            glVertex2f(sqX, sqY);
            glVertex2f(sqX + sqSize, sqY);
            glVertex2f(sqX + sqSize, sqY + sqSize);
            glVertex2f(sqX, sqY + sqSize);
            glEnd();
        } else if (obstacle.type == ObstacleType::PLATFORM) {
            // Draw platform hitbox (yellow)
            glColor3f(1.0f, 1.0f, 0.0f);  // yellow
            glBegin(GL_LINE_LOOP);
            glVertex2f(screenX, obstacle.y);
            glVertex2f(screenX + obstacle.w, obstacle.y);
            glVertex2f(screenX + obstacle.w, obstacle.y + obstacle.h);
            glVertex2f(screenX, obstacle.y + obstacle.h);
            glEnd();
        }
    }

    glEnable(GL_BLEND);  // Re-enable blending
}

void drawFpsCounter() {
    if (!SHOW_FPS) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_W, 0, WINDOW_H, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);  // white text

    // Create FPS string
    char fpsStr[32];
    sprintf(fpsStr, "FPS: %d", currentFps);
    renderString(10.0f, WINDOW_H - 30.0f, GLUT_BITMAP_HELVETICA_18, fpsStr);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
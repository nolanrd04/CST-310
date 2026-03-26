#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "constants.h"
#include "renderer.h"
#include "player.h"
#include "level.h"
#include "obstacle.h"

// Game state
enum GameState { MENU, PLAYING, WIN, DEAD };
GameState gameState = MENU;
Player player;
Level level;

void init() {
    glewExperimental = GL_TRUE;
    glewInit();

    initRenderer();
    srand(time(0));
    player.reset();
    level.generate(42);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw background and ground
    drawBackground();
    drawGround(0.0f);

    // Draw obstacles (spikes and platforms)
    drawObstacles(level.obstacles, level.cameraX);

    // Draw player cube with rotation and white outline
    drawQuadRotatedWithOutline(player.screenX(), player.y, player.w, player.h, player.rotation,
                               glm::vec3(PLAYER_R, PLAYER_G, PLAYER_B),
                               glm::vec3(1.0f, 1.0f, 1.0f),  // white outline
                               3.0f);  // 3 pixel outline

    // Draw player face (eyes and mouth)
    drawPlayerFace(player.screenX(), player.y, player.w, player.h, player.rotation);

    // Draw debug hitboxes
    drawHitboxes(player, level.obstacles, level.cameraX);

    // Draw HUD (game state messages)
    drawHUD(gameState);

    // Draw FPS counter
    drawFpsCounter();

    glutSwapBuffers();
}

void timer(int value) {
    // Update animation time and FPS counter always
    updateAnimationTime(DT);
    updateFpsCounter();

    if (gameState == PLAYING) {
        if (!player.onGround) {
            player.airRotationSpeed = computeAirRotationSpeed(player, level.obstacles, level.cameraX, GROUND_Y);
        }

        player.update(DT, GROUND_Y);
        level.update(DT);

        // Check collision with obstacles
        for (const auto& obs : level.obstacles) {
            if (obs.type == ObstacleType::SPIKE) {
                if (checkCollisionWithSpike(player, obs, level.cameraX)) {
                    gameState = DEAD;
                    break;
                }
            } else if (obs.type == ObstacleType::PLATFORM) {
                if (checkCollisionWithPlatform(player, obs, level.cameraX)) {
                    float platTop = obs.y + obs.h;
                    // Only land on platform if player was above it (not jumping through from below)
                    if (player.prevY >= platTop) {
                        player.y = obs.y + obs.h;
                        player.velY = 0;
                        player.onGround = true;
                        player.airRotationSpeed = ROTATION_SPEED;
                        // Snap rotation to nearest 90°
                        player.rotation = snapRotationTo90(player.rotation);
                        player.timeInAir = 0;
                    }
                }
            }
        }

        // Check win condition
        if (player.worldX >= LEVEL_LENGTH) {
            gameState = WIN;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // 16ms = ~60fps
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // ESC
        exit(0);
    }
    if (key == ' ') {  // SPACE
        if (gameState == MENU) {
            gameState = PLAYING;
            player.reset();
        } else if (gameState == PLAYING) {
            player.jump();
        }
    }
    if (key == 'r' || key == 'R') {
        gameState = MENU;
        player.reset();
        level.generate(rand());  // new seed for new level
    }
    if (key == 'h' || key == 'H') {  // Toggle hitbox debug
        extern bool DRAW_HITBOXES;
        DRAW_HITBOXES = !DRAW_HITBOXES;
    }
    if (key == 'f' || key == 'F') {  // Toggle FPS counter
        extern bool SHOW_FPS;
        SHOW_FPS = !SHOW_FPS;
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutCreateWindow("Geometry Dash");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}

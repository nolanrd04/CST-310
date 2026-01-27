/**
* 1. Set ups
* * 1.1 Set up camera
* * 1.2 Set up materials
* * 1.3 Set up lighting
* 2. Draw Object Functions
* * 2.1 Draw Ground Plane
* * 2.2 Draw Cube
* 3. Draw Scene 
* * 3.1 Draw Ground Plane
* * 3.2 Draw Cube
* 4. Draw Scene
* * 4.1 Draw Ground Plane
* * 4.2 Draw Cube
* 5. Display callback
* 6. Reshape callback
* 7. Keyboard callbacks
* 8. Keyboard special keys
* 
* 9. Initialize
* 10. Main
*/




#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>    // GLUT: OpenGL Utility Toolkit - handles windows, input, etc.
#endif

#include <cmath>        // For sin(), cos(), M_PI - used in camera calculations
#include <cstdio>       // For printf() - console output
#include <cstdlib>      // For exit() - program termination


GLfloat cameraX = 0.0f;
GLfloat cameraY = 5.0f;
GLfloat cameraZ = 15.0f;

GLfloat cameraAngleX = 0.0f;  // Pitch (up/down rotation)
GLfloat cameraAngleY = 0.0f;  // Yaw (left/right rotation)

const GLfloat MOVE_SPEED = 0.5f;    // Units per key press
const GLfloat ROTATE_SPEED = 2.0f;  // Degrees per key press

void setMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat shininess = 50.0f) {
    GLfloat ambient[] = { r * 0.2f, g * 0.2f, b * 0.2f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void setupLighting() {
    /**
     * Enable Lighting
     * ---------------
     * Turns on OpenGL's lighting calculations.
     * Without this, no lights will affect the scene.
     */
    glEnable(GL_LIGHTING);

    /**
     * Set Up a Single Light Source (GL_LIGHT0)
     * -----------------------------------------
     * This light simulates a directional light (like the sun).
     * Directional lights have parallel rays and no attenuation.
     */
    GLfloat lightPos[] = { 0.0f, 10.0f, 10.0f, 0.0f }; // Directional light
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // Ambient light
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse light
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Specular light

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glEnable(GL_LIGHT0); // Activate the light source
}


void drawGroundPlane()
{
    
    GLfloat planeSize = 20.0f;    // Half-width of the plane (total = 40 units)
    int divisions = 20;            // Number of tiles per side
    GLfloat step = planeSize * 2 / divisions;  // Size of each tile

    // Start with dark gray material
    setMaterial(0.3f, 0.3f, 0.3f, 10.0f);

    glBegin(GL_QUADS);

    /**
     * All ground tiles share the same upward-facing normal.
     * Since the ground is flat on Y=0, normal is (0, 1, 0).
     */
    glNormal3f(0.0f, 1.0f, 0.0f);

    // Loop through grid positions to create tiles
    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            // Calculate corner position of this tile
            GLfloat x = -planeSize + i * step;
            GLfloat z = -planeSize + j * step;

            /**
             * Checkerboard Pattern
             * --------------------
             * (i + j) % 2 alternates between 0 and 1:
             *   (0,0)=0, (0,1)=1, (0,2)=0, ...
             *   (1,0)=1, (1,1)=0, (1,2)=1, ...
             * This creates the classic checkerboard look.
             */
            if ((i + j) % 2 == 0) {
                setMaterial(0.4f, 0.4f, 0.4f, 10.0f);  // Light gray
            } else {
                setMaterial(0.2f, 0.2f, 0.2f, 10.0f);  // Dark gray
            }

            // Draw tile as a quad on the Y=0 plane
            glVertex3f(x, 0.0f, z);
            glVertex3f(x + step, 0.0f, z);
            glVertex3f(x + step, 0.0f, z + step);
            glVertex3f(x, 0.0f, z + step);
        }
    }
    glEnd();
}

void drawCube(GLfloat size) {
    GLfloat half = size / 2.0f;  // Half-size for centering at origin
    glBegin(GL_QUADS);

    /**
     * FRONT FACE (Z = +half)
     * ----------------------
     * This face is toward the viewer (positive Z direction).
     * Normal points outward: (0, 0, 1)
     */
    glNormal3f(0.0f, 0.0f, 1.0f);           // Normal points toward viewer
    glVertex3f(-half, -half,  half);         // Bottom-left
    glVertex3f( half, -half,  half);         // Bottom-right
    glVertex3f( half,  half,  half);         // Top-right
    glVertex3f(-half,  half,  half);         // Top-left

    /**
     * BACK FACE (Z = -half)
     * Normal points outward (away from viewer): (0, 0, -1)
     */
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half,  half, -half);
    glVertex3f( half,  half, -half);
    glVertex3f( half, -half, -half);

    /**
     * TOP FACE (Y = +half)
     * --------------------
     * Normal points upward: (0, 1, 0)
     */
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-half,  half, -half);
    glVertex3f(-half,  half,  half);
    glVertex3f( half,  half,  half);
    glVertex3f( half,  half, -half);

    /**
     * BOTTOM FACE (Y = -half)
     * -----------------------
     * Normal points downward: (0, -1, 0)
     */
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f( half, -half, -half);
    glVertex3f( half, -half,  half);
    glVertex3f(-half, -half,  half);

    /**
     * RIGHT FACE (X = +half)
     * ----------------------
     * Normal points right: (1, 0, 0)
     */
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( half, -half, -half);
    glVertex3f( half,  half, -half);
    glVertex3f( half,  half,  half);
    glVertex3f( half, -half,  half);

    /**
     * LEFT FACE (X = -half)
     * ---------------------
     * Normal points left: (-1, 0, 0)
     */
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half,  half);
    glVertex3f(-half,  half,  half);
    glVertex3f(-half,  half, -half);

    glEnd();
}

void drawScene()
{
    // Draw ground plane first (at Y = 0)
    drawGroundPlane();
    // Draw a cube at the origin
    glPushMatrix();
        glTranslatef(0.0f, 1.0f, -10.0f);  // Move cube up so it sits on ground
        glScalef(10.0f, 10.0f, 1.0f);  // Make cube taller
        setMaterial(255/255.0f, 245/255.0f, 227/255.0f);  // Light tan material
        drawCube(2.0f);                  // Draw 2-unit cube
    glPopMatrix();
}

void display() 
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Camera Look Direction Calculation
    GLfloat lookX = cameraX + cosf(cameraAngleX * M_PI / 180.0f) * sinf(cameraAngleY * M_PI / 180.0f);
    GLfloat lookY = cameraY + sinf(cameraAngleX * M_PI / 180.0f);
    GLfloat lookZ = cameraZ - cosf(cameraAngleX * M_PI / 180.0f) * cosf(cameraAngleY * M_PI / 180.0f);
    gluLookAt(cameraX, cameraY, cameraZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);

    setupLighting();
    drawScene();
    glutSwapBuffers();
}

void reshape(int width, int height) 
{
    if (height == 0) height = 1; // Prevent division by zero
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

void keyboard(unsigned char key, int x, int y) 
{
    
    GLfloat radY = cameraAngleY * M_PI / 180.0f;
    switch (key) {
        
        // move position
        case 'w':
        case 'W':
            cameraX += sin(radY) * MOVE_SPEED;
            cameraZ -= cos(radY) * MOVE_SPEED;
            break;
        case 's':
        case 'S':
            cameraX -= sin(radY) * MOVE_SPEED;
            cameraZ += cos(radY) * MOVE_SPEED;
            break;
        case 'a':
        case 'A':
            cameraX -= cos(radY) * MOVE_SPEED;
            cameraZ -= sin(radY) * MOVE_SPEED;
            break;
        case 'd':
        case 'D':
            cameraX += cos(radY) * MOVE_SPEED;
            cameraZ += sin(radY) * MOVE_SPEED;
            break;

        // move height
        case 'q':
        case 'Q':
            cameraY += MOVE_SPEED;
            break;
        case 'e':
        case 'E':
            cameraY -= MOVE_SPEED;
            break;

        // reset position
        case 'r':
        case 'R':
            cameraX = 0.0f;
            cameraY = 5.0f;
            cameraZ = 15.0f;
            cameraAngleX = 0.0f;
            cameraAngleY = 0.0f;
            break;
        case 27: // ESC key
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) 
{
    switch (key) {
        case GLUT_KEY_UP:
            cameraAngleX += ROTATE_SPEED;
            if (cameraAngleX > 89.0f) cameraAngleX = 89.0f; // Prevent flipping
            break;
        case GLUT_KEY_DOWN:
            cameraAngleX -= ROTATE_SPEED;
            if (cameraAngleX < -89.0f) cameraAngleX = -89.0f; // Prevent flipping
            break;
        case GLUT_KEY_LEFT:
            cameraAngleY -= ROTATE_SPEED;
            break;
        case GLUT_KEY_RIGHT:
            cameraAngleY += ROTATE_SPEED;
            break;
    }
    glutPostRedisplay();
}

void init()
{
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Sky blue background
    glDepthFunc(GL_LEQUAL); // Type of depth test
    glShadeModel(GL_SMOOTH); // Smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective corrections
}

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Scene with Camera and Lighting");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}

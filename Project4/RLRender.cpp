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

#include <SOIL/SOIL.h>   // For loading textures

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

GLuint windowTexture;

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
    GLfloat lightPos[] = { 0.0f, 2.0f, 1.0f, 0.0f }; // Directional light
    GLfloat lightAmbient[] = { 0.9f, 0.9f, 0.9f, 0.0f }; // Ambient light
    GLfloat lightDiffuse[] = { 1.3f, 1.3f, 1.3f, 1.0f }; // Diffuse light
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

void drawWindows(int rows, int cols,
                 GLfloat buildingX, GLfloat buildingY, GLfloat buildingZ,
                 GLfloat buildingW, GLfloat buildingH, GLfloat buildingD,
                 GLfloat offsetX = 0.0f, GLfloat offsetY = 0.0f,
                 GLfloat spacingX = 0.5f, GLfloat spacingY = 0.5f,
                 GLfloat winWidth = 0.0f, GLfloat winHeight = 0.0f,
                 GLfloat matR = 0.3f, GLfloat matG = 0.5f, GLfloat matB = 0.8f) {
    // Margin from building edges (in world units)
    GLfloat marginX = buildingW * 0.05f;
    GLfloat marginY = buildingH * 0.05f;

    // Window area bounds in world space on the front face
    GLfloat left   = buildingX - buildingW + marginX + offsetX;
    GLfloat right  = buildingX + buildingW - marginX + offsetX;
    GLfloat bottom = buildingY - buildingH + marginY + buildingH * 0.1f + offsetY; // Extra bottom margin
    GLfloat top    = buildingY + buildingH - marginY + offsetY;

    GLfloat totalWidth  = right - left;
    GLfloat totalHeight = top - bottom;

    // Compute window size from available space if not specified
    GLfloat computedW = (totalWidth  - spacingX * (cols - 1)) / cols;
    GLfloat computedH = (totalHeight - spacingY * (rows - 1)) / rows;

    // Use specified size if provided, otherwise use computed size
    if (winWidth  <= 0.0f) winWidth  = computedW;
    if (winHeight <= 0.0f) winHeight = computedH;

    // Center the window grid within the available area
    GLfloat gridW = cols * winWidth  + (cols - 1) * spacingX;
    GLfloat gridH = rows * winHeight + (rows - 1) * spacingY;
    GLfloat startX = buildingX - gridW / 2.0f + offsetX;
    GLfloat startY = buildingY - gridH / 2.0f + offsetY;

    // Slightly in front of the building face to avoid z-fighting
    GLfloat frontZ = buildingZ + buildingD + 0.01f;

    glPushMatrix();
    glLoadIdentity();
    // Reapply the view matrix (camera) but no building scale
    GLfloat lookX = cameraX + cosf(cameraAngleX * M_PI / 180.0f) * sinf(cameraAngleY * M_PI / 180.0f);
    GLfloat lookY = cameraY + sinf(cameraAngleX * M_PI / 180.0f);
    GLfloat lookZ = cameraZ - cosf(cameraAngleX * M_PI / 180.0f) * cosf(cameraAngleY * M_PI / 180.0f);
    gluLookAt(cameraX, cameraY, cameraZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);

    setMaterial(matR, matG, matB, 80.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, windowTexture);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            GLfloat x1 = startX + c * (winWidth + spacingX);
            GLfloat x2 = x1 + winWidth;
            GLfloat y1 = startY + r * (winHeight + spacingY);
            GLfloat y2 = y1 + winHeight;

            // Tile the texture based on window size
            // texScale controls how many world units one texture copy covers
            GLfloat texScale = 1.0f;
            GLfloat texU = winWidth / texScale;
            GLfloat texV = winHeight / texScale;

            glTexCoord2f(0.0f, texV); glVertex3f(x1, y1, frontZ);
            glTexCoord2f(texU, texV); glVertex3f(x2, y1, frontZ);
            glTexCoord2f(texU, 0.0f); glVertex3f(x2, y2, frontZ);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y2, frontZ);
        }
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawScene()
{
    // Draw ground plane first (at Y = 0)
    drawGroundPlane();

    // ******** Building ******** //
    // Building parameters (world space)
    GLfloat bx = 0.0f, by = 3.25f, bz = -10.0f; // Position
    GLfloat sx = 10.0f, sy = 6.5f, sz = 1.0f;   // Scale
    GLfloat cubeSize = 2.0f;
    GLfloat halfCube = cubeSize / 2.0f;

    // World-space half-extents of the building
    GLfloat buildingW = halfCube * sx; // 10
    GLfloat buildingH = halfCube * sy; // 6.5
    GLfloat buildingD = halfCube * sz; // 1

    // Draw the building cube
    glPushMatrix();
        glTranslatef(bx, by, bz);
        glScalef(sx, sy, sz);
        setMaterial(255/255.0f, 245/255.0f, 227/255.0f);
        drawCube(cubeSize);
    glPopMatrix();

    // Draw windows independently in world space (no non-uniform scale)
    // winWidth=2.0, winHeight=2.0 -> square windows in world units
    // rows, cols = 1, 7 for vertical arrangement
    // buildingW, buildingH, buildingD for proper placement
    // bx, by, bz for building position
    // offsetX=0.0f, offsetY=0.0f for centered grid
    // spacingX=0.5f, spacingY=2.0f for spacing between windows
    // maxSizeX, maxSizeY = 2.0f to force square windows
    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, 3.2f, 0.08f, 0.08f, 2.0f, 2.3);

    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, 1.4f, 0.08f, 0.08f, 2.0f, 0.7);

    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, 0.6f, 0.08f, 0.08f, 2.0f, 0.7);

    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, -1.0f, 0.08f, 0.08f, 2.0f, 2.3);

    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, -3.6f, 0.08f, 0.08f, 2.0f, 2.3);

    

    // ******** Building Roof ******** //
    bx = 0.0f, by = 9.75f, bz = -10.0f; // Position on top of building
    sx = 10.0f, sy = 0.25f, sz = 4.0f;   // Scale
    cubeSize = 2.0f;

    // Draw the roof cube
    glPushMatrix();
        glTranslatef(bx, by + halfCube * sy, bz); // Position on top of building
        glScalef(sx, sy, sz);
        setMaterial(255/255.0f, 245/255.0f, 227/255.0f); // Red roof
        drawCube(cubeSize);
    glPopMatrix();

    // ******** Second Building Roof ******** //
    bx = 0.0f, by = 10.0f, bz = -10.0f; // Position on top of building
    sx = 10.0f, sy = 0.15f, sz = 4.0f;   // Scale
    cubeSize = 2.0f;

    // Draw the roof cube
    glPushMatrix();
        glTranslatef(bx, by + halfCube * sy, bz); // Position on top of building
        glScalef(sx, sy, sz);
        setMaterial(65/255.0f, 65/255.0f, 65/255.0f); // Red roof
        drawCube(cubeSize);
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

GLuint loadTexture(const char* filename) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height;
    unsigned char* image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
    if (!image) {
        printf("Failed to load texture: %s\n", filename);
        return 0;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void init()
{
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Sky blue background
    glDepthFunc(GL_LEQUAL); // Type of depth test
    glShadeModel(GL_SMOOTH); // Smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective corrections
    glEnable(GL_NORMALIZE); // Normalize normals for scaled objects

    windowTexture = loadTexture("window_texture.png");
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

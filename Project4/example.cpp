/**
 * OpenGL 3D Scene Example
 *
 * This program demonstrates fundamental OpenGL concepts for rendering 3D scenes:
 * - Multiple 3D primitives (cube, sphere, cone, cylinder, torus)
 * - Camera positioning with gluLookAt
 * - Basic lighting (ambient, diffuse, specular)
 * - Material properties
 * - Keyboard controls for camera movement
 * - Transformations (translate, rotate, scale)
 *
 * Controls:
 *   W/S - Move camera forward/backward
 *   A/D - Move camera left/right
 *   Q/E - Move camera up/down
 *   Arrow keys - Rotate view
 *   R - Reset camera to initial position
 *   ESC - Exit program
 */

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>

// ============================================================================
// CAMERA VARIABLES
// ============================================================================
// Camera position in world space
GLfloat cameraX = 0.0f;
GLfloat cameraY = 5.0f;
GLfloat cameraZ = 15.0f;

// Camera rotation angles (in degrees)
GLfloat cameraAngleX = 0.0f;  // Pitch (up/down)
GLfloat cameraAngleY = 0.0f;  // Yaw (left/right)

// Movement speed
const GLfloat MOVE_SPEED = 0.5f;
const GLfloat ROTATE_SPEED = 2.0f;

// ============================================================================
// LIGHTING SETUP
// ============================================================================
/**
 * Initializes OpenGL lighting.
 *
 * OpenGL supports up to 8 lights (GL_LIGHT0 through GL_LIGHT7).
 * Each light has several properties:
 * - Ambient: Background light that illuminates all surfaces equally
 * - Diffuse: Directional light that creates shading based on surface angle
 * - Specular: Creates shiny highlights on surfaces
 * - Position: Where the light is located (w=0 for directional, w=1 for point)
 */
void setupLighting() {
    // Enable lighting calculations
    glEnable(GL_LIGHTING);

    // Enable the first light source
    glEnable(GL_LIGHT0);

    // Define light properties
    // Light position: (x, y, z, w) - w=1.0 means positional light
    GLfloat lightPosition[] = { 5.0f, 10.0f, 5.0f, 1.0f };

    // Ambient light - low intensity background illumination
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    // Diffuse light - main directional lighting
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };

    // Specular light - for shiny highlights
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Apply light properties
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // Enable automatic normal normalization
    // This ensures lighting calculations remain correct after scaling transforms
    glEnable(GL_NORMALIZE);
}

// ============================================================================
// MATERIAL FUNCTIONS
// ============================================================================
/**
 * Sets the current material color for objects.
 *
 * Materials define how surfaces interact with light:
 * - Ambient: How much ambient light the surface reflects
 * - Diffuse: The main color of the surface
 * - Specular: Color of shiny highlights
 * - Shininess: How focused the specular highlight is (0-128)
 */
void setMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat shininess = 50.0f) {
    GLfloat ambient[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

// ============================================================================
// PRIMITIVE DRAWING FUNCTIONS
// ============================================================================

/**
 * Draws a cube manually using GL_QUADS.
 * This demonstrates how to define vertices and normals for each face.
 *
 * A cube has 6 faces, each with 4 vertices and 1 normal vector.
 * The normal vector points perpendicular to the face for lighting.
 */
void drawCube(GLfloat size) {
    GLfloat half = size / 2.0f;

    glBegin(GL_QUADS);

    // Front face (normal pointing toward viewer: 0, 0, 1)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-half, -half,  half);
    glVertex3f( half, -half,  half);
    glVertex3f( half,  half,  half);
    glVertex3f(-half,  half,  half);

    // Back face (normal pointing away: 0, 0, -1)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half,  half, -half);
    glVertex3f( half,  half, -half);
    glVertex3f( half, -half, -half);

    // Top face (normal pointing up: 0, 1, 0)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-half,  half, -half);
    glVertex3f(-half,  half,  half);
    glVertex3f( half,  half,  half);
    glVertex3f( half,  half, -half);

    // Bottom face (normal pointing down: 0, -1, 0)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f( half, -half, -half);
    glVertex3f( half, -half,  half);
    glVertex3f(-half, -half,  half);

    // Right face (normal pointing right: 1, 0, 0)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( half, -half, -half);
    glVertex3f( half,  half, -half);
    glVertex3f( half,  half,  half);
    glVertex3f( half, -half,  half);

    // Left face (normal pointing left: -1, 0, 0)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half,  half);
    glVertex3f(-half,  half,  half);
    glVertex3f(-half,  half, -half);

    glEnd();
}

/**
 * Draws a ground plane (grid) to provide spatial reference.
 * This helps visualize the 3D space and camera movement.
 */
void drawGroundPlane() {
    GLfloat planeSize = 20.0f;
    int divisions = 20;
    GLfloat step = planeSize * 2 / divisions;

    // Set ground material to dark gray
    setMaterial(0.3f, 0.3f, 0.3f, 10.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            GLfloat x = -planeSize + i * step;
            GLfloat z = -planeSize + j * step;

            // Checkerboard pattern
            if ((i + j) % 2 == 0) {
                setMaterial(0.4f, 0.4f, 0.4f, 10.0f);
            } else {
                setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
            }

            glVertex3f(x, 0.0f, z);
            glVertex3f(x + step, 0.0f, z);
            glVertex3f(x + step, 0.0f, z + step);
            glVertex3f(x, 0.0f, z + step);
        }
    }
    glEnd();
}

/**
 * Draws the complete 3D scene with multiple objects.
 *
 * Each object demonstrates different concepts:
 * - Translation: Moving objects to different positions
 * - Rotation: Spinning objects around axes
 * - Scaling: Changing object sizes
 * - Different primitives: Showing variety of shapes
 */
void drawScene() {
    // Draw ground plane first
    drawGroundPlane();

    // ========================================
    // Object 1: Red Cube (manually drawn)
    // Demonstrates: glPushMatrix/glPopMatrix, translation, custom geometry
    // ========================================
    glPushMatrix();                          // Save current transformation
        glTranslatef(-4.0f, 1.5f, 0.0f);     // Move to position
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);  // Rotate 45 degrees around Y axis
        setMaterial(0.8f, 0.2f, 0.2f);       // Red material
        drawCube(2.0f);                       // Draw 2x2x2 cube
    glPopMatrix();                           // Restore transformation

    // ========================================
    // Object 2: Green Sphere (GLUT primitive)
    // Demonstrates: glutSolidSphere, smooth shading
    // ========================================
    glPushMatrix();
        glTranslatef(0.0f, 2.0f, 0.0f);
        setMaterial(0.2f, 0.8f, 0.2f);
        // glutSolidSphere(radius, slices, stacks)
        // More slices/stacks = smoother sphere
        glutSolidSphere(1.5, 32, 32);
    glPopMatrix();

    // ========================================
    // Object 3: Blue Cone (GLUT primitive)
    // Demonstrates: glutSolidCone, vertical orientation
    // ========================================
    glPushMatrix();
        glTranslatef(4.0f, 0.0f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Rotate to point upward
        setMaterial(0.2f, 0.2f, 0.8f);
        // glutSolidCone(base_radius, height, slices, stacks)
        glutSolidCone(1.0, 3.0, 32, 8);
    glPopMatrix();

    // ========================================
    // Object 4: Yellow Torus (donut shape)
    // Demonstrates: glutSolidTorus
    // ========================================
    glPushMatrix();
        glTranslatef(-4.0f, 1.0f, -5.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        setMaterial(0.8f, 0.8f, 0.2f);
        // glutSolidTorus(inner_radius, outer_radius, sides, rings)
        glutSolidTorus(0.3, 1.0, 16, 32);
    glPopMatrix();

    // ========================================
    // Object 5: Cyan Cylinder (built from disk + tube)
    // Demonstrates: combining primitives, GLU quadrics
    // ========================================
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        setMaterial(0.2f, 0.8f, 0.8f);

        // Create a quadric object for cylinder
        GLUquadric* quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);

        // Draw cylinder (base_radius, top_radius, height, slices, stacks)
        gluCylinder(quad, 1.0, 1.0, 2.5, 32, 8);

        // Draw bottom cap
        gluDisk(quad, 0.0, 1.0, 32, 1);

        // Draw top cap
        glTranslatef(0.0f, 0.0f, 2.5f);
        gluDisk(quad, 0.0, 1.0, 32, 1);

        gluDeleteQuadric(quad);
    glPopMatrix();

    // ========================================
    // Object 6: Magenta Teapot (classic OpenGL test object)
    // Demonstrates: glutSolidTeapot
    // ========================================
    glPushMatrix();
        glTranslatef(4.0f, 1.0f, -5.0f);
        setMaterial(0.8f, 0.2f, 0.8f);
        glutSolidTeapot(1.0);
    glPopMatrix();

    // ========================================
    // Object 7: Orange Scaled Cube (demonstrates scaling)
    // Demonstrates: non-uniform scaling
    // ========================================
    glPushMatrix();
        glTranslatef(-6.0f, 1.0f, 3.0f);
        glScalef(1.0f, 2.0f, 0.5f);  // Scale: normal X, tall Y, thin Z
        setMaterial(1.0f, 0.5f, 0.0f);
        glutSolidCube(1.5);
    glPopMatrix();

    // ========================================
    // Object 8: White Icosahedron (20-sided polyhedron)
    // Demonstrates: glutSolidIcosahedron
    // ========================================
    glPushMatrix();
        glTranslatef(6.0f, 1.5f, 3.0f);
        setMaterial(0.9f, 0.9f, 0.9f);
        glScalef(1.5f, 1.5f, 1.5f);
        glutSolidIcosahedron();
    glPopMatrix();
}

// ============================================================================
// DISPLAY CALLBACK
// ============================================================================
/**
 * Main display function called by GLUT whenever the window needs redrawing.
 *
 * The rendering pipeline:
 * 1. Clear buffers (color and depth)
 * 2. Set up the modelview matrix
 * 3. Position the camera with gluLookAt
 * 4. Draw all objects in the scene
 * 5. Swap buffers (for double buffering)
 */
void display() {
    // Clear the color buffer and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Switch to modelview matrix mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Calculate camera look direction based on rotation angles
    // Convert angles to radians for trigonometry
    GLfloat radX = cameraAngleX * M_PI / 180.0f;
    GLfloat radY = cameraAngleY * M_PI / 180.0f;

    // Calculate the point the camera is looking at
    GLfloat lookX = cameraX + sin(radY) * cos(radX);
    GLfloat lookY = cameraY + sin(radX);
    GLfloat lookZ = cameraZ - cos(radY) * cos(radX);

    /**
     * gluLookAt sets up the view transformation.
     * Parameters:
     *   eyeX, eyeY, eyeZ     - Camera position
     *   centerX, centerY, centerZ - Point to look at
     *   upX, upY, upZ        - Up direction vector
     */
    gluLookAt(
        cameraX, cameraY, cameraZ,    // Eye position
        lookX, lookY, lookZ,           // Look-at point
        0.0f, 1.0f, 0.0f              // Up vector (Y is up)
    );

    // Set up lighting (position updates with camera)
    setupLighting();

    // Draw the 3D scene
    drawScene();

    // Swap front and back buffers (double buffering)
    glutSwapBuffers();
}

// ============================================================================
// RESHAPE CALLBACK
// ============================================================================
/**
 * Called when the window is resized.
 * Sets up the projection matrix to maintain proper aspect ratio.
 */
void reshape(int width, int height) {
    // Prevent division by zero
    if (height == 0) height = 1;

    // Calculate aspect ratio
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set the viewport to cover the entire window
    glViewport(0, 0, width, height);

    // Set up the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /**
     * gluPerspective sets up a perspective projection.
     * Parameters:
     *   fovy   - Field of view angle in Y direction (degrees)
     *   aspect - Aspect ratio (width/height)
     *   zNear  - Distance to near clipping plane
     *   zFar   - Distance to far clipping plane
     *
     * Objects closer than zNear or farther than zFar won't be rendered.
     */
    gluPerspective(60.0f, aspect, 0.1f, 100.0f);

    // Switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================================
// KEYBOARD CALLBACKS
// ============================================================================
/**
 * Handles regular key presses (letters, numbers, etc.)
 */
void keyboard(unsigned char key, int x, int y) {
    // Calculate movement direction based on camera angle
    GLfloat radY = cameraAngleY * M_PI / 180.0f;

    switch (key) {
        // Forward/Backward movement (W/S)
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

        // Strafe left/right (A/D)
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

        // Move up/down (Q/E)
        case 'q':
        case 'Q':
            cameraY += MOVE_SPEED;
            break;
        case 'e':
        case 'E':
            cameraY -= MOVE_SPEED;
            break;

        // Reset camera (R)
        case 'r':
        case 'R':
            cameraX = 0.0f;
            cameraY = 5.0f;
            cameraZ = 15.0f;
            cameraAngleX = 0.0f;
            cameraAngleY = 0.0f;
            break;

        // Exit (ESC)
        case 27:  // ESC key
            exit(0);
            break;
    }

    // Request a redraw
    glutPostRedisplay();
}

/**
 * Handles special key presses (arrow keys, function keys, etc.)
 */
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            cameraAngleX += ROTATE_SPEED;
            if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;  // Clamp to prevent flipping
            break;
        case GLUT_KEY_DOWN:
            cameraAngleX -= ROTATE_SPEED;
            if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
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

// ============================================================================
// INITIALIZATION
// ============================================================================
/**
 * One-time OpenGL initialization.
 */
void init() {
    // Set background color to dark blue (like sky)
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // Enable depth testing (objects in front hide objects behind)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Enable smooth shading (colors interpolate across polygons)
    glShadeModel(GL_SMOOTH);

    // Set up nice perspective correction for textures
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);

    /**
     * Set display mode:
     * - GLUT_DOUBLE: Double buffering for smooth animation
     * - GLUT_RGB: RGB color mode
     * - GLUT_DEPTH: Depth buffer for 3D rendering
     */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Set initial window size and position
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    // Create the window with a title
    glutCreateWindow("OpenGL 3D Scene Example - Use WASD/QE to move, Arrows to look");

    // Register callback functions
    glutDisplayFunc(display);       // Called to draw the scene
    glutReshapeFunc(reshape);       // Called when window is resized
    glutKeyboardFunc(keyboard);     // Called for regular key presses
    glutSpecialFunc(specialKeys);   // Called for special keys (arrows, etc.)

    // Perform one-time initialization
    init();

    // Print controls to console
    printf("=== OpenGL 3D Scene Example ===\n");
    printf("Controls:\n");
    printf("  W/S     - Move forward/backward\n");
    printf("  A/D     - Strafe left/right\n");
    printf("  Q/E     - Move up/down\n");
    printf("  Arrows  - Look around\n");
    printf("  R       - Reset camera\n");
    printf("  ESC     - Exit\n");
    printf("================================\n");

    // Enter the main event loop (never returns)
    glutMainLoop();

    return 0;
}
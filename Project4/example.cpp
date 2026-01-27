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

/**
 * PREPROCESSOR DIRECTIVES
 * -----------------------
 * These #include statements bring in external library headers.
 * The #ifdef __APPLE_CC__ checks if we're compiling on macOS,
 * which uses a different path for GLUT headers.
 */
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>    // GLUT: OpenGL Utility Toolkit - handles windows, input, etc.
#endif

#include <cmath>        // For sin(), cos(), M_PI - used in camera calculations
#include <cstdio>       // For printf() - console output
#include <cstdlib>      // For exit() - program termination

// ============================================================================
// CAMERA VARIABLES
// ============================================================================
/**
 * Camera Position Variables
 * -------------------------
 * These define WHERE the camera is located in 3D world space.
 * OpenGL uses a right-handed coordinate system:
 *   - X axis: positive = right, negative = left
 *   - Y axis: positive = up, negative = down
 *   - Z axis: positive = toward viewer, negative = into screen
 *
 * Initial position (0, 5, 15) places the camera:
 *   - Centered horizontally (X=0)
 *   - 5 units above the ground (Y=5)
 *   - 15 units back from origin (Z=15), looking toward the scene
 */
GLfloat cameraX = 0.0f;
GLfloat cameraY = 5.0f;
GLfloat cameraZ = 15.0f;

/**
 * Camera Rotation Angles (in degrees)
 * ------------------------------------
 * These control WHERE the camera is looking.
 *   - cameraAngleX (Pitch): Looking up (+) or down (-)
 *   - cameraAngleY (Yaw): Looking left (-) or right (+)
 *
 * These angles are converted to radians and used with sin/cos
 * to calculate the look-at point for gluLookAt().
 */
GLfloat cameraAngleX = 0.0f;  // Pitch (up/down rotation)
GLfloat cameraAngleY = 0.0f;  // Yaw (left/right rotation)

/**
 * Movement Speed Constants
 * ------------------------
 * These control how fast the camera moves and rotates.
 * Adjust these values to make movement faster or slower.
 */
const GLfloat MOVE_SPEED = 0.5f;    // Units per key press
const GLfloat ROTATE_SPEED = 2.0f;  // Degrees per key press

// ============================================================================
// LIGHTING SETUP
// ============================================================================
/**
 * setupLighting()
 * ---------------
 * Initializes OpenGL's lighting system.
 *
 * OpenGL's fixed-function pipeline supports up to 8 simultaneous lights
 * (GL_LIGHT0 through GL_LIGHT7). Each light can have different properties
 * that determine how it illuminates objects in the scene.
 *
 * LIGHTING MODEL OVERVIEW:
 * The final color of a pixel is calculated as:
 *   FinalColor = Emission + Ambient + Diffuse + Specular
 *
 * Where each component is: LightProperty × MaterialProperty
 *
 * This function sets up one positional light (like a light bulb)
 * positioned above and to the side of the scene.
 */
void setupLighting() {
    /**
     * glEnable(GL_LIGHTING)
     * ---------------------
     * Turns ON OpenGL's lighting calculations.
     * Without this, objects are drawn with their vertex colors only,
     * ignoring all light sources. With this enabled, OpenGL calculates
     * how light interacts with surface normals and materials.
     */
    glEnable(GL_LIGHTING);

    /**
     * glEnable(GL_LIGHT0)
     * -------------------
     * Activates the first light source.
     * OpenGL has 8 lights (GL_LIGHT0 to GL_LIGHT7).
     * Each must be individually enabled to contribute to the scene.
     * You can have multiple lights with different colors/positions.
     */
    glEnable(GL_LIGHT0);

    /**
     * Light Position Array
     * --------------------
     * Format: { x, y, z, w }
     *
     * The 'w' component determines the light TYPE:
     *   w = 1.0 : POSITIONAL light (like a light bulb)
     *             - Has a specific location in space
     *             - Light rays radiate outward from that point
     *             - Objects closer to the light are brighter
     *
     *   w = 0.0 : DIRECTIONAL light (like the sun)
     *             - Infinitely far away
     *             - All light rays are parallel
     *             - x, y, z become the DIRECTION vector
     *
     * Position (5, 10, 5) places the light:
     *   - 5 units to the right
     *   - 10 units above (high up, like ceiling light)
     *   - 5 units toward the viewer
     */
    GLfloat lightPosition[] = { 5.0f, 10.0f, 5.0f, 1.0f };

    /**
     * Ambient Light Component
     * -----------------------
     * Format: { red, green, blue, alpha } (values 0.0 to 1.0)
     *
     * Ambient light simulates indirect/bounced light that fills a room.
     * It illuminates ALL surfaces equally, regardless of their orientation.
     * There are no shadows from ambient light alone.
     *
     * Low values (0.2) create a dim background illumination,
     * ensuring objects aren't completely black on their unlit sides.
     */
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    /**
     * Diffuse Light Component
     * -----------------------
     * Format: { red, green, blue, alpha } (values 0.0 to 1.0)
     *
     * Diffuse light is the main directional lighting.
     * It creates shading based on the angle between:
     *   - The surface normal (which way the surface faces)
     *   - The direction to the light source
     *
     * Surfaces facing the light are bright.
     * Surfaces facing away are dark.
     * This is what creates the 3D appearance of objects.
     *
     * High values (0.8) make this the dominant light component.
     */
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };

    /**
     * Specular Light Component
     * ------------------------
     * Format: { red, green, blue, alpha } (values 0.0 to 1.0)
     *
     * Specular light creates shiny highlights on glossy surfaces.
     * The highlight appears where light reflects directly toward the viewer.
     * The size/sharpness of the highlight depends on the material's shininess.
     *
     * White specular (1.0, 1.0, 1.0) creates realistic highlights
     * that work well with any material color.
     */
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    /**
     * glLightfv() - Set Light Properties
     * ----------------------------------
     * Parameters:
     *   light    : Which light to modify (GL_LIGHT0 to GL_LIGHT7)
     *   pname    : Which property to set (GL_POSITION, GL_AMBIENT, etc.)
     *   params   : Array of values for that property
     *
     * The 'fv' suffix means:
     *   f = float values
     *   v = vector (array) of values
     */
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    /**
     * glEnable(GL_NORMALIZE)
     * ----------------------
     * Tells OpenGL to automatically normalize all normal vectors.
     *
     * Normal vectors MUST be unit length (length = 1.0) for correct
     * lighting calculations. However, when you apply glScalef(),
     * the normals get scaled too, making them non-unit length.
     *
     * GL_NORMALIZE fixes this by renormalizing after transformations.
     * It has a small performance cost but ensures correct lighting.
     */
    glEnable(GL_NORMALIZE);
}

// ============================================================================
// MATERIAL FUNCTIONS
// ============================================================================
/**
 * setMaterial()
 * -------------
 * Sets the current material properties for subsequently drawn objects.
 *
 * Parameters:
 *   r, g, b    : The base color of the material (0.0 to 1.0 each)
 *   shininess  : How shiny/glossy the surface is (0.0 to 128.0)
 *                - 0 = completely matte (no specular highlight)
 *                - 128 = mirror-like (tiny, intense highlight)
 *                - 50 = moderately shiny (good default)
 *
 * MATERIAL PROPERTIES EXPLAINED:
 *
 * Materials define how a surface REFLECTS light. The final color is:
 *   Ambient × LightAmbient + Diffuse × LightDiffuse + Specular × LightSpecular
 *
 * This function sets:
 *   - Ambient:  Darker version of the color (r*0.3, g*0.3, b*0.3)
 *               This is the color in shadowed areas
 *   - Diffuse:  The main color (r, g, b)
 *               This is the color in lit areas
 *   - Specular: White (1, 1, 1)
 *               Highlights are white regardless of object color
 *   - Shininess: Controls specular highlight size
 */
void setMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat shininess = 50.0f) {
    /**
     * Material property arrays
     * All use RGBA format: { red, green, blue, alpha }
     * Alpha is typically 1.0 (fully opaque)
     */
    GLfloat ambient[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    /**
     * glMaterialfv() - Set Material Vector Properties
     * ------------------------------------------------
     * Parameters:
     *   face   : Which face(s) to apply to
     *            - GL_FRONT: front-facing polygons only
     *            - GL_BACK: back-facing polygons only
     *            - GL_FRONT_AND_BACK: both sides
     *   pname  : Which property (GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR)
     *   params : Array of RGBA values
     */
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

    /**
     * glMaterialf() - Set Material Scalar Property
     * ---------------------------------------------
     * Parameters:
     *   face   : Which face(s) to apply to
     *   pname  : GL_SHININESS (the only scalar material property)
     *   param  : Shininess value (0.0 to 128.0)
     *
     * The 'f' suffix (no 'v') means single float value, not array.
     */
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

// ============================================================================
// PRIMITIVE DRAWING FUNCTIONS
// ============================================================================

/**
 * drawCube()
 * ----------
 * Draws a cube centered at the origin using immediate mode (glBegin/glEnd).
 *
 * Parameters:
 *   size : The length of each edge of the cube
 *
 * This function demonstrates MANUAL geometry creation:
 * - Each face is a quad (4 vertices)
 * - Each face needs a NORMAL vector for lighting
 * - Vertices are specified in counter-clockwise order (front-facing)
 *
 * VERTEX WINDING ORDER:
 * OpenGL uses the "right-hand rule" to determine which side of a polygon
 * is the "front". If you curl your right hand fingers in the order
 * vertices are specified, your thumb points toward the front.
 * Front faces are typically rendered; back faces can be culled.
 *
 * NORMAL VECTORS:
 * A normal is a vector perpendicular to a surface. It tells OpenGL
 * which direction the surface is facing, which is essential for lighting.
 * For a cube, each face has one normal pointing straight out from it.
 */
void drawCube(GLfloat size) {
    GLfloat half = size / 2.0f;  // Half-size for centering at origin

    /**
     * glBegin(GL_QUADS)
     * -----------------
     * Starts defining geometry. GL_QUADS means every 4 vertices
     * form a quadrilateral (4-sided polygon).
     *
     * Other common modes:
     *   GL_TRIANGLES : Every 3 vertices form a triangle
     *   GL_POINTS    : Each vertex is a point
     *   GL_LINES     : Every 2 vertices form a line
     *   GL_POLYGON   : All vertices form one polygon
     */
    glBegin(GL_QUADS);

    /**
     * FRONT FACE (Z = +half)
     * ----------------------
     * This face is toward the viewer (positive Z direction).
     * Normal points outward: (0, 0, 1)
     *
     * glNormal3f(x, y, z)
     * -------------------
     * Sets the normal vector for subsequent vertices.
     * The normal applies to all vertices until a new normal is set.
     * Parameters are the x, y, z components of the normal vector.
     *
     * glVertex3f(x, y, z)
     * -------------------
     * Specifies a vertex position in 3D space.
     * Vertices are specified counter-clockwise when viewed from front.
     */
    glNormal3f(0.0f, 0.0f, 1.0f);           // Normal points toward viewer
    glVertex3f(-half, -half,  half);         // Bottom-left
    glVertex3f( half, -half,  half);         // Bottom-right
    glVertex3f( half,  half,  half);         // Top-right
    glVertex3f(-half,  half,  half);         // Top-left

    /**
     * BACK FACE (Z = -half)
     * ---------------------
     * This face is away from the viewer (negative Z direction).
     * Normal points outward (away from viewer): (0, 0, -1)
     * Vertices are still counter-clockwise when viewed from OUTSIDE the cube.
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

    /**
     * glEnd()
     * -------
     * Signals the end of vertex specification.
     * Must be called after glBegin() to complete the geometry.
     */
    glEnd();
}

/**
 * drawGroundPlane()
 * -----------------
 * Draws a flat checkerboard ground plane for spatial reference.
 *
 * This helps the viewer understand:
 * - Where objects are positioned in the scene
 * - How the camera is moving
 * - The scale of the environment
 *
 * The checkerboard pattern is created by alternating colors
 * based on tile position (i + j) % 2.
 */
void drawGroundPlane() {
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

/**
 * drawScene()
 * -----------
 * Draws all objects in the 3D scene.
 *
 * This function demonstrates the TRANSFORMATION PIPELINE:
 *
 * IMPORTANT CONCEPT - Matrix Stack:
 * ---------------------------------
 * OpenGL maintains a "matrix stack" for transformations.
 *
 * glPushMatrix() : Saves the current transformation state (pushes a copy onto the stack)
 * glPopMatrix()  : Restores the previous state (pops from the stack)
 *
 * This allows you to:
 * 1. Save the current state
 * 2. Apply transforms for ONE object
 * 3. Draw that object
 * 4. Restore the state (removing those transforms)
 * 5. Start fresh for the next object
 *
 * Without Push/Pop, transforms would accumulate and affect all subsequent objects!
 *
 * TRANSFORMATION ORDER:
 * --------------------
 * Transformations are applied in REVERSE order of how they're written!
 *
 * If you write:
 *   glTranslatef(5, 0, 0);   // Written first
 *   glRotatef(45, 0, 1, 0);  // Written second
 *   drawCube();
 *
 * The cube is: Rotated FIRST, then Translated.
 *
 * Think of it as: "Move the coordinate system, then draw in local coordinates"
 * Or mathematically: vertex' = Translation × Rotation × vertex
 */
void drawScene() {
    // Draw ground plane first (at Y = 0)
    drawGroundPlane();

    // ========================================
    // OBJECT 1: Red Cube (custom geometry)
    // ========================================
    /**
     * This demonstrates:
     * - glPushMatrix/glPopMatrix for isolated transforms
     * - glTranslatef for positioning
     * - glRotatef for rotation
     * - Custom cube drawing with normals
     */
    glPushMatrix();
        /**
         * glTranslatef(x, y, z)
         * ---------------------
         * Moves (translates) subsequent geometry by (x, y, z).
         * This moves the cube to position (-4, 1.5, 0):
         *   - 4 units to the left
         *   - 1.5 units up (so it sits on the ground)
         *   - Centered on Z axis
         */
        glTranslatef(-4.0f, 1.5f, 0.0f);

        /**
         * glRotatef(angle, x, y, z)
         * -------------------------
         * Rotates subsequent geometry by 'angle' degrees
         * around the axis defined by (x, y, z).
         *
         * Parameters:
         *   angle : Rotation amount in degrees (counter-clockwise when
         *           looking down the axis toward origin)
         *   x,y,z : The axis of rotation (will be normalized)
         *
         * Common rotations:
         *   (45, 0, 1, 0) : Rotate 45° around Y axis (like spinning a top)
         *   (90, 1, 0, 0) : Rotate 90° around X axis (like nodding head)
         *   (30, 0, 0, 1) : Rotate 30° around Z axis (like tilting head)
         */
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);

        setMaterial(0.8f, 0.2f, 0.2f);  // Red material
        drawCube(2.0f);                  // Draw 2-unit cube
    glPopMatrix();  // Restore transformation state

    // ========================================
    // OBJECT 2: Green Sphere (GLUT primitive)
    // ========================================
    /**
     * This demonstrates:
     * - Using GLUT's built-in sphere primitive
     * - How to control sphere smoothness
     */
    glPushMatrix();
        glTranslatef(0.0f, 2.0f, 0.0f);  // Center of scene, 2 units up
        setMaterial(0.2f, 0.8f, 0.2f);    // Green material

        /**
         * glutSolidSphere(radius, slices, stacks)
         * ---------------------------------------
         * Draws a solid sphere centered at the origin.
         *
         * Parameters:
         *   radius : Size of the sphere
         *   slices : Number of vertical divisions (longitude lines)
         *            Like the segments of an orange
         *   stacks : Number of horizontal divisions (latitude lines)
         *            Like the layers of an onion
         *
         * More slices/stacks = smoother sphere, but more polygons.
         * Typical values: 16-32 for good quality.
         *
         * The sphere automatically has correct normals for lighting.
         */
        glutSolidSphere(1.5, 32, 32);
    glPopMatrix();

    // ========================================
    // OBJECT 3: Blue Cone (GLUT primitive)
    // ========================================
    /**
     * This demonstrates:
     * - glutSolidCone
     * - Rotating to change orientation (cone normally points along +Z)
     */
    glPushMatrix();
        glTranslatef(4.0f, 0.0f, 0.0f);  // Right side of scene, on ground

        /**
         * The cone is created pointing along the +Z axis by default.
         * We rotate -90° around X to make it point UP (+Y direction).
         *
         * Why -90? Using right-hand rule:
         * - Point thumb along +X axis
         * - Fingers curl from +Y toward +Z
         * - So positive rotation moves +Z toward +Y
         * - We want opposite: +Z toward -Y, so use negative angle
         */
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        setMaterial(0.2f, 0.2f, 0.8f);  // Blue material

        /**
         * glutSolidCone(base, height, slices, stacks)
         * -------------------------------------------
         * Draws a cone with base on Z=0 plane, pointing toward +Z.
         *
         * Parameters:
         *   base   : Radius of the circular base
         *   height : Height of the cone (length along Z axis)
         *   slices : Number of divisions around the circumference
         *   stacks : Number of divisions along the height
         */
        glutSolidCone(1.0, 3.0, 32, 8);
    glPopMatrix();

    // ========================================
    // OBJECT 4: Yellow Torus (donut shape)
    // ========================================
    /**
     * This demonstrates:
     * - glutSolidTorus (donut/ring shape)
     * - The torus lies flat by default; we rotate to show it better
     */
    glPushMatrix();
        glTranslatef(-4.0f, 1.0f, -5.0f);  // Back left of scene
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Lay flat
        setMaterial(0.8f, 0.8f, 0.2f);  // Yellow material

        /**
         * glutSolidTorus(innerRadius, outerRadius, sides, rings)
         * -------------------------------------------------------
         * Draws a torus (donut shape) centered at origin.
         *
         * Parameters:
         *   innerRadius : Radius of the tube itself
         *   outerRadius : Distance from center to middle of tube
         *   sides       : Number of faces around each ring cross-section
         *   rings       : Number of rings around the torus
         *
         * Think of it as a circle (radius=innerRadius) swept around
         * a larger circle (radius=outerRadius).
         */
        glutSolidTorus(0.3, 1.0, 16, 32);
    glPopMatrix();

    // ========================================
    // OBJECT 5: Cyan Cylinder (GLU quadric)
    // ========================================
    /**
     * This demonstrates:
     * - GLU quadric objects
     * - Building a cylinder with caps
     * - gluCylinder and gluDisk
     */
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -5.0f);  // Center back
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);  // Stand upright
        setMaterial(0.2f, 0.8f, 0.8f);  // Cyan material

        /**
         * GLU Quadric Objects
         * -------------------
         * GLU (OpenGL Utility Library) provides "quadric" objects
         * for drawing common shapes like cylinders, spheres, disks.
         *
         * gluNewQuadric()
         * ---------------
         * Creates a new quadric object. Returns a pointer used for
         * subsequent quadric operations.
         */
        GLUquadric* quad = gluNewQuadric();

        /**
         * gluQuadricNormals(quad, mode)
         * -----------------------------
         * Sets how normals are generated for the quadric.
         *
         * Parameters:
         *   quad : The quadric object
         *   mode : Normal generation mode
         *          - GLU_NONE   : No normals (no lighting)
         *          - GLU_FLAT   : One normal per face (faceted look)
         *          - GLU_SMOOTH : Normals interpolated (smooth shading)
         */
        gluQuadricNormals(quad, GLU_SMOOTH);

        /**
         * gluCylinder(quad, base, top, height, slices, stacks)
         * ----------------------------------------------------
         * Draws a cylinder (or cone if base != top) along the Z axis.
         *
         * Parameters:
         *   quad   : The quadric object
         *   base   : Radius at Z=0
         *   top    : Radius at Z=height (0 for a cone)
         *   height : Length along Z axis
         *   slices : Divisions around circumference
         *   stacks : Divisions along length
         *
         * NOTE: gluCylinder is HOLLOW - no end caps!
         * We need to draw disks separately for closed ends.
         */
        gluCylinder(quad, 1.0, 1.0, 2.5, 32, 8);

        /**
         * gluDisk(quad, inner, outer, slices, loops)
         * ------------------------------------------
         * Draws a flat circular disk on Z=0 plane.
         *
         * Parameters:
         *   quad   : The quadric object
         *   inner  : Inner radius (0 for solid disk, >0 for ring)
         *   outer  : Outer radius
         *   slices : Divisions around circumference
         *   loops  : Concentric divisions (usually 1)
         */
        // Bottom cap (at Z=0, we're already there)
        gluDisk(quad, 0.0, 1.0, 32, 1);

        // Top cap (need to move to Z=height first)
        glTranslatef(0.0f, 0.0f, 2.5f);
        gluDisk(quad, 0.0, 1.0, 32, 1);

        /**
         * gluDeleteQuadric(quad)
         * ----------------------
         * Frees memory used by the quadric object.
         * Always delete quadrics when done to prevent memory leaks.
         */
        gluDeleteQuadric(quad);
    glPopMatrix();

    // ========================================
    // OBJECT 6: Magenta Teapot
    // ========================================
    /**
     * This demonstrates:
     * - The classic Utah Teapot, a standard test object in computer graphics
     */
    glPushMatrix();
        glTranslatef(4.0f, 1.0f, -5.0f);  // Back right
        setMaterial(0.8f, 0.2f, 0.8f);    // Magenta material

        /**
         * glutSolidTeapot(size)
         * ---------------------
         * Draws the famous Utah Teapot.
         *
         * Parameters:
         *   size : Approximate radius of the teapot
         *
         * The Utah Teapot was created in 1975 and has become
         * a standard test model in computer graphics, like
         * "Hello World" is for programming.
         */
        glutSolidTeapot(1.0);
    glPopMatrix();

    // ========================================
    // OBJECT 7: Orange Scaled Cube
    // ========================================
    /**
     * This demonstrates:
     * - glScalef for non-uniform scaling
     * - Creating elongated/flattened shapes from basic primitives
     */
    glPushMatrix();
        glTranslatef(-6.0f, 1.0f, 3.0f);  // Front left

        /**
         * glScalef(x, y, z)
         * -----------------
         * Scales subsequent geometry by the given factors.
         *
         * Parameters:
         *   x : Scale factor along X axis (1.0 = no change)
         *   y : Scale factor along Y axis
         *   z : Scale factor along Z axis
         *
         * Examples:
         *   (2, 2, 2)    : Make everything twice as big
         *   (1, 2, 1)    : Stretch to double height
         *   (0.5, 1, 1)  : Squish to half width
         *   (-1, 1, 1)   : Mirror/flip along X axis
         *
         * This creates a tall, thin box from a cube.
         */
        glScalef(1.0f, 2.0f, 0.5f);  // Normal X, tall Y, thin Z

        setMaterial(1.0f, 0.5f, 0.0f);  // Orange material

        /**
         * glutSolidCube(size)
         * -------------------
         * Draws a solid cube centered at origin.
         * Unlike our custom drawCube, this uses GLUT's built-in cube.
         *
         * Parameters:
         *   size : Length of each edge
         */
        glutSolidCube(1.5);
    glPopMatrix();

    // ========================================
    // OBJECT 8: White Icosahedron
    // ========================================
    /**
     * This demonstrates:
     * - glutSolidIcosahedron (20-sided regular polyhedron)
     * - Scaling to adjust size (icosahedron has fixed unit radius)
     */
    glPushMatrix();
        glTranslatef(6.0f, 1.5f, 3.0f);  // Front right
        setMaterial(0.9f, 0.9f, 0.9f);    // White material

        /**
         * Scale up because glutSolidIcosahedron has a fixed size
         * (vertices are approximately 1 unit from center)
         */
        glScalef(1.5f, 1.5f, 1.5f);

        /**
         * glutSolidIcosahedron()
         * ----------------------
         * Draws a regular icosahedron (20 triangular faces).
         * Centered at origin with radius approximately 1.
         *
         * No parameters - it's a fixed shape.
         *
         * Other GLUT platonic solids:
         *   glutSolidTetrahedron()  - 4 faces
         *   glutSolidOctahedron()   - 8 faces
         *   glutSolidDodecahedron() - 12 faces
         *   glutSolidIcosahedron()  - 20 faces
         */
        glutSolidIcosahedron();
    glPopMatrix();
}

// ============================================================================
// DISPLAY CALLBACK
// ============================================================================
/**
 * display()
 * ---------
 * The main rendering function, called by GLUT whenever the window needs redrawing.
 *
 * This is called:
 * - Initially when the window is first shown
 * - When the window is resized
 * - When glutPostRedisplay() is called (e.g., after keyboard input)
 * - When the window is uncovered after being hidden
 *
 * THE RENDERING PIPELINE:
 * ----------------------
 * 1. Clear buffers (remove previous frame)
 * 2. Set up modelview matrix (camera + world transforms)
 * 3. Draw objects (geometry with materials)
 * 4. Swap buffers (display the new frame)
 */
void display() {
    /**
     * glClear(mask)
     * -------------
     * Clears one or more buffers to their preset clear values.
     *
     * Parameters (bitwise OR together):
     *   GL_COLOR_BUFFER_BIT : Clear the color buffer (the image)
     *                         Uses color set by glClearColor()
     *   GL_DEPTH_BUFFER_BIT : Clear the depth buffer (Z-buffer)
     *                         Resets all depths to maximum (far plane)
     *
     * The depth buffer tracks how far each pixel is from the camera.
     * Clearing it ensures new objects aren't hidden by old depth data.
     */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /**
     * glMatrixMode(mode)
     * ------------------
     * Selects which matrix stack subsequent operations affect.
     *
     * Parameters:
     *   GL_MODELVIEW  : Combined model and view transformations
     *                   (object positions and camera position)
     *   GL_PROJECTION : The projection transformation
     *                   (perspective or orthographic)
     *   GL_TEXTURE    : Texture coordinate transformations
     */
    glMatrixMode(GL_MODELVIEW);

    /**
     * glLoadIdentity()
     * ----------------
     * Resets the current matrix to the identity matrix.
     *
     * The identity matrix is like "no transformation":
     * [ 1 0 0 0 ]
     * [ 0 1 0 0 ]
     * [ 0 0 1 0 ]
     * [ 0 0 0 1 ]
     *
     * We reset before setting up the camera to start fresh each frame.
     */
    glLoadIdentity();

    /**
     * Camera Look Direction Calculation
     * ----------------------------------
     * We need to convert camera rotation angles into a "look-at" point
     * for gluLookAt(). This uses basic trigonometry.
     *
     * The camera looks in a direction based on:
     *   - cameraAngleY (yaw): Rotation around Y axis (left/right)
     *   - cameraAngleX (pitch): Rotation around X axis (up/down)
     *
     * We calculate a point 1 unit away in the look direction.
     */

    // Convert angles from degrees to radians (trig functions use radians)
    // Radians = Degrees × (π / 180)
    GLfloat radX = cameraAngleX * M_PI / 180.0f;
    GLfloat radY = cameraAngleY * M_PI / 180.0f;

    /**
     * Calculate look-at point using spherical coordinates:
     *
     * For a unit sphere:
     *   x = sin(yaw) × cos(pitch)   - horizontal offset
     *   y = sin(pitch)               - vertical offset
     *   z = -cos(yaw) × cos(pitch)  - depth offset (negative because
     *                                  camera looks toward -Z by default)
     *
     * Add camera position to get world-space look-at point.
     */
    GLfloat lookX = cameraX + sin(radY) * cos(radX);
    GLfloat lookY = cameraY + sin(radX);
    GLfloat lookZ = cameraZ - cos(radY) * cos(radX);

    /**
     * gluLookAt() - Set Up the Camera View
     * ------------------------------------
     * Creates a viewing matrix and multiplies it onto the current matrix.
     *
     * Parameters:
     *   eyeX, eyeY, eyeZ       : Camera POSITION in world coordinates
     *                            Where is the camera located?
     *
     *   centerX, centerY, centerZ : LOOK-AT POINT in world coordinates
     *                               What point is the camera looking at?
     *
     *   upX, upY, upZ          : UP VECTOR
     *                            Which direction is "up" for the camera?
     *                            Usually (0, 1, 0) meaning Y-axis is up.
     *
     * The camera looks FROM eye TOWARD center, with up defining the
     * camera's orientation (prevents rolling).
     *
     * Example:
     *   gluLookAt(0, 5, 10,    // Camera at (0, 5, 10)
     *             0, 0, 0,     // Looking at origin
     *             0, 1, 0);    // Y is up
     */
    gluLookAt(
        cameraX, cameraY, cameraZ,    // Eye position (where camera is)
        lookX, lookY, lookZ,           // Look-at point (where camera looks)
        0.0f, 1.0f, 0.0f              // Up vector (Y is up)
    );

    // Set up lighting (position is affected by current modelview matrix)
    setupLighting();

    // Draw all objects in the scene
    drawScene();

    /**
     * glutSwapBuffers()
     * -----------------
     * Swaps the front and back buffers (for double buffering).
     *
     * DOUBLE BUFFERING EXPLAINED:
     * Without double buffering, you'd see objects being drawn one by one,
     * causing flickering. With double buffering:
     *
     * 1. Draw everything to the BACK buffer (invisible)
     * 2. When done, swap: back becomes front (visible), front becomes back
     * 3. Start drawing the next frame to the new back buffer
     *
     * This ensures users only see complete frames, never partial ones.
     *
     * Requires GLUT_DOUBLE in glutInitDisplayMode().
     */
    glutSwapBuffers();
}

// ============================================================================
// RESHAPE CALLBACK
// ============================================================================
/**
 * reshape()
 * ---------
 * Called when the window is resized.
 *
 * Parameters:
 *   width  : New window width in pixels
 *   height : New window height in pixels
 *
 * This function:
 * 1. Updates the viewport to cover the new window size
 * 2. Recalculates the projection matrix with correct aspect ratio
 *
 * Without proper reshape handling, the scene would be stretched/squished
 * when the window is resized.
 */
void reshape(int width, int height) {
    // Prevent division by zero if window height becomes 0
    if (height == 0) height = 1;

    /**
     * Aspect Ratio
     * ------------
     * The ratio of width to height. Used to prevent distortion.
     *
     * A 800×600 window has aspect ratio 800/600 = 1.33
     * A 1920×1080 window has aspect ratio 1920/1080 = 1.78
     *
     * Objects should appear the same shape regardless of window size.
     */
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    /**
     * glViewport(x, y, width, height)
     * -------------------------------
     * Defines the region of the window where OpenGL renders.
     *
     * Parameters:
     *   x, y   : Lower-left corner of viewport (usually 0, 0)
     *   width  : Width in pixels
     *   height : Height in pixels
     *
     * The viewport maps the normalized device coordinates (-1 to 1)
     * to actual pixel coordinates on screen.
     */
    glViewport(0, 0, width, height);

    // Switch to projection matrix to set up the camera "lens"
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /**
     * gluPerspective() - Set Up Perspective Projection
     * -------------------------------------------------
     * Creates a perspective projection matrix.
     *
     * Parameters:
     *   fovy   : Field of View in Y direction (vertical), in degrees
     *            - 45-60° is typical for most applications
     *            - Larger values = wider angle (more visible, more distortion)
     *            - Smaller values = telephoto effect (zoomed in)
     *
     *   aspect : Aspect ratio (width / height)
     *            - Prevents horizontal stretching/squishing
     *            - Should match the window's aspect ratio
     *
     *   zNear  : Distance to the NEAR clipping plane
     *            - Objects closer than this are NOT rendered
     *            - Must be > 0 (can't be AT the camera)
     *            - Smaller values = can see things up close
     *            - Too small causes Z-fighting (flickering)
     *
     *   zFar   : Distance to the FAR clipping plane
     *            - Objects farther than this are NOT rendered
     *            - Larger values = can see farther
     *            - zFar/zNear ratio affects depth precision
     *
     * The VIEW FRUSTUM is the visible region:
     * A pyramid with its tip at the camera, cut by near/far planes.
     *
     *         Near plane         Far plane
     *              |                  |
     *     Camera   |                  |
     *        *-----+------------------+
     *              |\                /|
     *              | \   visible    / |
     *              |  \   region   /  |
     *              | / \          / \ |
     *              |/   \________/   \|
     *              +------------------+
     */
    gluPerspective(60.0f, aspect, 0.1f, 100.0f);

    // Switch back to modelview matrix for rendering
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================================
// KEYBOARD CALLBACKS
// ============================================================================
/**
 * keyboard()
 * ----------
 * Handles regular key presses (letters, numbers, symbols).
 *
 * Parameters:
 *   key : ASCII value of the key pressed
 *   x   : Mouse X position when key was pressed (often unused)
 *   y   : Mouse Y position when key was pressed (often unused)
 *
 * This function implements WASD-style movement:
 * - W/S : Move forward/backward
 * - A/D : Strafe left/right
 * - Q/E : Move up/down
 *
 * Movement is relative to camera direction (controlled by cameraAngleY).
 */
void keyboard(unsigned char key, int x, int y) {
    /**
     * Calculate movement direction based on camera yaw.
     * This makes W always move "forward" relative to where you're looking.
     *
     * sin(radY) gives the X component of the forward direction
     * cos(radY) gives the Z component of the forward direction
     */
    GLfloat radY = cameraAngleY * M_PI / 180.0f;

    switch (key) {
        /**
         * FORWARD MOVEMENT (W)
         * --------------------
         * Move in the direction the camera is facing.
         *
         * Forward direction in XZ plane:
         *   X component: sin(yaw)
         *   Z component: -cos(yaw)  (negative because camera looks toward -Z)
         */
        case 'w':
        case 'W':
            cameraX += sin(radY) * MOVE_SPEED;
            cameraZ -= cos(radY) * MOVE_SPEED;
            break;

        /**
         * BACKWARD MOVEMENT (S)
         * ---------------------
         * Move opposite to the forward direction.
         */
        case 's':
        case 'S':
            cameraX -= sin(radY) * MOVE_SPEED;
            cameraZ += cos(radY) * MOVE_SPEED;
            break;

        /**
         * STRAFE LEFT (A)
         * ---------------
         * Move perpendicular to forward direction (90° left).
         *
         * Left direction:
         *   X component: -cos(yaw)
         *   Z component: -sin(yaw)
         */
        case 'a':
        case 'A':
            cameraX -= cos(radY) * MOVE_SPEED;
            cameraZ -= sin(radY) * MOVE_SPEED;
            break;

        /**
         * STRAFE RIGHT (D)
         * ----------------
         * Move perpendicular to forward direction (90° right).
         */
        case 'd':
        case 'D':
            cameraX += cos(radY) * MOVE_SPEED;
            cameraZ += sin(radY) * MOVE_SPEED;
            break;

        /**
         * MOVE UP (Q)
         * -----------
         * Increase Y position (fly upward).
         */
        case 'q':
        case 'Q':
            cameraY += MOVE_SPEED;
            break;

        /**
         * MOVE DOWN (E)
         * -------------
         * Decrease Y position (fly downward).
         */
        case 'e':
        case 'E':
            cameraY -= MOVE_SPEED;
            break;

        /**
         * RESET CAMERA (R)
         * ----------------
         * Return camera to initial position and orientation.
         */
        case 'r':
        case 'R':
            cameraX = 0.0f;
            cameraY = 5.0f;
            cameraZ = 15.0f;
            cameraAngleX = 0.0f;
            cameraAngleY = 0.0f;
            break;

        /**
         * EXIT (ESC)
         * ----------
         * Terminate the program.
         * ESC has ASCII value 27.
         */
        case 27:  // ESC key
            exit(0);
            break;
    }

    /**
     * glutPostRedisplay()
     * -------------------
     * Marks the current window as needing to be redisplayed.
     *
     * This doesn't immediately redraw - it sets a flag so that
     * the display callback will be called on the next iteration
     * of the GLUT main loop.
     *
     * Without this, the scene wouldn't update after moving!
     */
    glutPostRedisplay();
}

/**
 * specialKeys()
 * -------------
 * Handles special key presses (arrow keys, function keys, etc.).
 *
 * Parameters:
 *   key : GLUT constant for the special key (GLUT_KEY_UP, etc.)
 *   x   : Mouse X position when key was pressed
 *   y   : Mouse Y position when key was pressed
 *
 * Special keys don't have ASCII values, so they're handled separately.
 * This function uses arrow keys to rotate the camera view.
 */
void specialKeys(int key, int x, int y) {
    switch (key) {
        /**
         * LOOK UP (Up Arrow)
         * ------------------
         * Increase pitch angle (look upward).
         * Clamped to 89° to prevent flipping at vertical.
         */
        case GLUT_KEY_UP:
            cameraAngleX += ROTATE_SPEED;
            if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
            break;

        /**
         * LOOK DOWN (Down Arrow)
         * ----------------------
         * Decrease pitch angle (look downward).
         * Clamped to -89° to prevent flipping.
         */
        case GLUT_KEY_DOWN:
            cameraAngleX -= ROTATE_SPEED;
            if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
            break;

        /**
         * LOOK LEFT (Left Arrow)
         * ----------------------
         * Decrease yaw angle (turn left).
         * No clamping needed - can rotate 360°.
         */
        case GLUT_KEY_LEFT:
            cameraAngleY -= ROTATE_SPEED;
            break;

        /**
         * LOOK RIGHT (Right Arrow)
         * ------------------------
         * Increase yaw angle (turn right).
         */
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
 * init()
 * ------
 * One-time OpenGL state initialization.
 *
 * This function sets up OpenGL state that persists throughout the program.
 * Called once after the window is created, before the main loop starts.
 */
void init() {
    /**
     * glClearColor(red, green, blue, alpha)
     * -------------------------------------
     * Sets the color used when clearing the color buffer.
     *
     * Parameters:
     *   red, green, blue : Color components (0.0 to 1.0)
     *   alpha            : Transparency (usually 1.0 for opaque)
     *
     * This sets a dark blue background, like a night sky.
     * This color is used by glClear(GL_COLOR_BUFFER_BIT).
     */
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    /**
     * glEnable(GL_DEPTH_TEST)
     * -----------------------
     * Enables depth testing (Z-buffering).
     *
     * Without depth testing, objects are drawn in the order you call them,
     * so objects drawn later appear in front even if they're farther away.
     *
     * With depth testing, each pixel stores a depth value.
     * New pixels only overwrite if they're closer to the camera.
     * This creates correct occlusion (near objects hide far objects).
     */
    glEnable(GL_DEPTH_TEST);

    /**
     * glDepthFunc(func)
     * -----------------
     * Sets the comparison function for depth testing.
     *
     * Parameters:
     *   GL_LESS   : Pass if new depth < stored depth (default)
     *   GL_LEQUAL : Pass if new depth <= stored depth
     *   GL_EQUAL  : Pass if new depth == stored depth
     *   GL_GREATER: Pass if new depth > stored depth
     *   GL_ALWAYS : Always pass (disables depth test effect)
     *
     * GL_LEQUAL is slightly more robust than GL_LESS for coplanar surfaces.
     */
    glDepthFunc(GL_LEQUAL);

    /**
     * glShadeModel(mode)
     * ------------------
     * Sets how colors are interpolated across polygons.
     *
     * Parameters:
     *   GL_SMOOTH : Colors are interpolated smoothly (Gouraud shading)
     *               Creates smooth gradients across surfaces
     *   GL_FLAT   : Each polygon is one solid color
     *               Uses color of last vertex in the polygon
     *               Creates faceted/low-poly look
     */
    glShadeModel(GL_SMOOTH);

    /**
     * glHint(target, mode)
     * --------------------
     * Provides hints to OpenGL for quality vs. speed tradeoffs.
     *
     * Parameters:
     *   target : Which operation to hint
     *            - GL_PERSPECTIVE_CORRECTION_HINT : Texture/color interpolation
     *            - GL_LINE_SMOOTH_HINT : Anti-aliased lines
     *            - GL_FOG_HINT : Fog calculation
     *   mode   : Quality preference
     *            - GL_FASTEST : Prefer speed over quality
     *            - GL_NICEST  : Prefer quality over speed
     *            - GL_DONT_CARE : No preference
     *
     * These are hints, not commands - the driver may ignore them.
     */
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
/**
 * main()
 * ------
 * Program entry point. Sets up GLUT and starts the main loop.
 *
 * Parameters:
 *   argc : Number of command-line arguments
 *   argv : Array of command-line argument strings
 *
 * GLUT can parse some command-line arguments (like -display on X11).
 */
int main(int argc, char** argv) {
    /**
     * glutInit(&argc, argv)
     * ---------------------
     * Initializes the GLUT library.
     *
     * Must be called before any other GLUT functions.
     * It processes and removes GLUT-specific command-line arguments.
     * Pass addresses so GLUT can modify argc/argv.
     */
    glutInit(&argc, argv);

    /**
     * glutInitDisplayMode(mode)
     * -------------------------
     * Sets the initial display mode for windows.
     *
     * Parameters (bitwise OR together):
     *   GLUT_SINGLE : Single buffering (not recommended, causes flicker)
     *   GLUT_DOUBLE : Double buffering (smooth animation)
     *   GLUT_RGB    : RGB color mode (vs. indexed color)
     *   GLUT_RGBA   : RGB with alpha channel
     *   GLUT_DEPTH  : Depth buffer (required for depth testing)
     *   GLUT_STENCIL: Stencil buffer (for advanced effects)
     *
     * GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH is the standard for 3D graphics.
     */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    /**
     * glutInitWindowSize(width, height)
     * ---------------------------------
     * Sets the initial window size in pixels.
     *
     * This is a request - the window manager may override it.
     * The actual size may differ (check with glutGet(GLUT_WINDOW_WIDTH)).
     */
    glutInitWindowSize(800, 600);

    /**
     * glutInitWindowPosition(x, y)
     * ----------------------------
     * Sets the initial window position on screen.
     *
     * (0, 0) is typically the top-left corner of the screen.
     * This is also just a request to the window manager.
     */
    glutInitWindowPosition(100, 100);

    /**
     * glutCreateWindow(title)
     * -----------------------
     * Creates a top-level window with the given title.
     *
     * Returns a window identifier (useful for multiple windows).
     * The window isn't displayed until glutMainLoop() is called.
     */
    glutCreateWindow("OpenGL 3D Scene Example - Use WASD/QE to move, Arrows to look");

    /**
     * CALLBACK REGISTRATION
     * ---------------------
     * GLUT uses callbacks to handle events. You register functions that
     * GLUT calls when specific events occur.
     *
     * glutDisplayFunc(func)
     *   Called whenever the window needs to be redrawn.
     *   This is where your rendering code goes.
     *
     * glutReshapeFunc(func)
     *   Called when the window is resized.
     *   Update viewport and projection here.
     *
     * glutKeyboardFunc(func)
     *   Called when a regular key (letter, number, etc.) is pressed.
     *
     * glutSpecialFunc(func)
     *   Called when a special key (arrow, F1-F12, etc.) is pressed.
     *
     * Other useful callbacks (not used here):
     *   glutMouseFunc      : Mouse button clicks
     *   glutMotionFunc     : Mouse movement while button held
     *   glutPassiveMotionFunc : Mouse movement without buttons
     *   glutIdleFunc       : Called when nothing else is happening
     *   glutTimerFunc      : Called after a specified delay
     */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    // Perform one-time OpenGL initialization
    init();

    // Print controls to console for user reference
    printf("=== OpenGL 3D Scene Example ===\n");
    printf("Controls:\n");
    printf("  W/S     - Move forward/backward\n");
    printf("  A/D     - Strafe left/right\n");
    printf("  Q/E     - Move up/down\n");
    printf("  Arrows  - Look around\n");
    printf("  R       - Reset camera\n");
    printf("  ESC     - Exit\n");
    printf("================================\n");

    /**
     * glutMainLoop()
     * --------------
     * Enters the GLUT event processing loop.
     *
     * This function NEVER RETURNS. It continuously:
     * 1. Checks for events (keyboard, mouse, window, etc.)
     * 2. Calls appropriate callbacks
     * 3. Redraws the window when needed
     *
     * To exit, call exit() from a callback (like we do with ESC).
     */
    glutMainLoop();

    // This line is never reached due to glutMainLoop()
    return 0;
}
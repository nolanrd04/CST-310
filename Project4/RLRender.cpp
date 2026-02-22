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


GLfloat cameraX = -1.0f;
GLfloat cameraY = 3.25f;
GLfloat cameraZ = 15.95f;

GLfloat cameraAngleX = 0.0f;  // Pitch (up/down rotation)
GLfloat cameraAngleY = 0.0f;  // Yaw (left/right rotation)

const GLfloat MOVE_SPEED = 0.5f;    // Units per key press
const GLfloat ROTATE_SPEED = 2.0f;  // Degrees per key press

GLuint windowTexture;
int windowWidth = 800;
int windowHeight = 600;

struct Color {
    GLfloat r, g, b;
};

struct WindowStyle {
    GLfloat r, g, b;       // Top/main color
    GLfloat splitRatio;    // 1.0 = solid color, 0.7 = top 70% / bottom 30%
    GLfloat r2, g2, b2;    // Bottom color (used when splitRatio < 1.0)
};

// Helper: solid color window (no split)
WindowStyle solid(Color c) {
    WindowStyle s = { c.r, c.g, c.b, 1.0f, c.r, c.g, c.b };
    return s;
}

// Helper: split window with top and bottom colors
WindowStyle split(Color top, GLfloat ratio, Color bot) {
    WindowStyle s = { top.r, top.g, top.b, ratio, bot.r, bot.g, bot.b };
    return s;
}

// window colors
Color winColor1 = { 137.0f/255.0f, 144.0f/255.0f, 196.0f/255.0f };  // Dark Blue
Color winColor2 = { 65.0f/255.0f, 67.0f/255.0f, 82.0f/255.0f }; // Almost black
Color winColor3 = { 201.0f/255.0f, 206.0f/255.0f, 242.0f/255.0f }; // gray
Color winColor4 = { 201.0f/255.0f, 242.0f/255.0f, 233.0f/255.0f };
Color winColor5 = {155.0f/255.0f, 189.0f/255.0f, 181.0f/255.0f};

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

void drawBitmapText(const char* text, GLfloat x, GLfloat y, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }
}

void drawCameraCoordinatesOverlay() {
    char coordinateText[96];
    std::snprintf(
        coordinateText,
        sizeof(coordinateText),
        "Camera: X %.2f  Y %.2f  Z %.2f",
        cameraX, cameraY, cameraZ
    );

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<GLdouble>(windowWidth), 0.0, static_cast<GLdouble>(windowHeight));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw a light shadow under the text so it stays readable on bright areas.
    glColor3f(0.95f, 0.95f, 0.95f);
    drawBitmapText(coordinateText, 11.0f, static_cast<GLfloat>(windowHeight - 19));

    glColor3f(0.0f, 0.0f, 0.0f);
    drawBitmapText(coordinateText, 10.0f, static_cast<GLfloat>(windowHeight - 20));

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}


void drawGroundPlane()
{
    
    GLfloat planeSizeX = 30.0f;    // Half-width of the plane (total = 40 units)
    GLfloat planeSizeZ = 30.0f;    // Half-depth of the plane (total = 40 units)
    int divisions = 20;            // Number of tiles per side
    GLfloat stepX = planeSizeX * 2 / divisions;  // Size of each tile in X
    GLfloat stepZ = planeSizeZ * 2 / divisions;  // Size of each tile in Z

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
            GLfloat x = -planeSizeX + i * stepX;
            GLfloat z = -planeSizeZ + j * stepZ;

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

            // Draw tile as a quad on the Y= -5.5f plane
            glVertex3f(x, -5.5f, z);
            glVertex3f(x + stepX, -5.5f, z);
            glVertex3f(x + stepX, -5.5f, z + stepZ);
            glVertex3f(x, -5.5f, z + stepZ);
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

void drawWindowFrame(GLfloat centerX, GLfloat centerY, GLfloat frontFaceZ,
                     GLfloat frameWidth, GLfloat frameHeight,
                     GLfloat frameDepth, GLfloat borderThickness,
                     GLfloat dividerThickness) {
    GLfloat halfW = frameWidth * 0.5f;
    GLfloat halfH = frameHeight * 0.5f;
    GLfloat centerZ = frontFaceZ + frameDepth * 0.5f;
    GLfloat innerHeight = frameHeight - (2.0f * borderThickness);

    if (innerHeight < borderThickness) {
        innerHeight = borderThickness;
    }

    // Frame color similar to the metal/aluminum look in your reference image.
    setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 30.0f);

    // Left vertical bar
    glPushMatrix();
        glTranslatef(centerX - halfW + borderThickness * 0.5f, centerY, centerZ);
        glScalef(borderThickness, frameHeight, frameDepth);
        drawCube(1.0f);
    glPopMatrix();

    // Right vertical bar
    glPushMatrix();
        glTranslatef(centerX + halfW - borderThickness * 0.5f, centerY, centerZ);
        glScalef(borderThickness, frameHeight, frameDepth);
        drawCube(1.0f);
    glPopMatrix();

    // Top horizontal bar
    glPushMatrix();
        glTranslatef(centerX, centerY + halfH - borderThickness * 0.5f, centerZ);
        glScalef(frameWidth, borderThickness, frameDepth);
        drawCube(1.0f);
    glPopMatrix();

    // Bottom horizontal bar
    glPushMatrix();
        glTranslatef(centerX, centerY - halfH + borderThickness * 0.5f, centerZ);
        glScalef(frameWidth, borderThickness, frameDepth);
        drawCube(1.0f);
    glPopMatrix();

    // Center divider (gives the two-panel frame look)
    glPushMatrix();
        glTranslatef(centerX, centerY, centerZ);
        glScalef(dividerThickness, innerHeight, frameDepth);
        drawCube(1.0f);
    glPopMatrix();
}

void drawWindowTextureOverlay(GLfloat centerX, GLfloat centerY, GLfloat frontFaceZ,
                              GLfloat frameWidth, GLfloat frameHeight,
                              GLfloat frameDepth, GLfloat alpha,
                              GLfloat forwardOffset,
                              GLfloat texVTop = 0.0f,
                              GLfloat texVBottom = 1.0f) {
    GLfloat halfW = frameWidth * 0.5f;
    GLfloat halfH = frameHeight * 0.5f;
    GLfloat overlayZ = frontFaceZ + frameDepth + forwardOffset;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                 GL_CURRENT_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, windowTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Keep depth testing on so the pane still respects occluders,
    // but do not write depth for this transparent pass.
    glDepthMask(GL_FALSE);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    GLfloat texScale = 1.0f;
    GLfloat texU = (halfW * 2.0f) / texScale;
    GLfloat texV = (halfH * 2.0f) / texScale;
    GLfloat texVTopScaled = texVTop * texV;
    GLfloat texVBottomScaled = texVBottom * texV;

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, texVTopScaled);    glVertex3f(centerX - halfW, centerY + halfH, overlayZ);
        glTexCoord2f(texU,  texVTopScaled);    glVertex3f(centerX + halfW, centerY + halfH, overlayZ);
        glTexCoord2f(texU,  texVBottomScaled); glVertex3f(centerX + halfW, centerY - halfH, overlayZ);
        glTexCoord2f(0.0f, texVBottomScaled); glVertex3f(centerX - halfW, centerY - halfH, overlayZ);
    glEnd();

    glDepthMask(GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
}

void drawSphere(GLfloat cx, GLfloat cy, GLfloat cz,
                GLfloat r, int slices, int stacks) {
    for (int i = 0; i < stacks; i++) {
        GLfloat lat0 = M_PI * (-0.5f + (GLfloat)i / stacks);
        GLfloat lat1 = M_PI * (-0.5f + (GLfloat)(i + 1) / stacks);
        GLfloat y0 = sinf(lat0);
        GLfloat y1 = sinf(lat1);
        GLfloat r0 = cosf(lat0);
        GLfloat r1 = cosf(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            GLfloat lng = 2.0f * M_PI * j / slices;
            GLfloat lx = cosf(lng);
            GLfloat lz = sinf(lng);

            glNormal3f(lx * r0, y0, lz * r0);
            glVertex3f(cx + r * lx * r0, cy + r * y0, cz + r * lz * r0);
            glNormal3f(lx * r1, y1, lz * r1);
            glVertex3f(cx + r * lx * r1, cy + r * y1, cz + r * lz * r1);
        }
        glEnd();
    }
}

void drawDrawString(GLfloat topX, GLfloat topY, GLfloat topZ,
                    GLfloat length, GLfloat radius, int segments, int rings, bool drawKnob) {
    // Chain of sphere beads from topY down to topY - length.
    setMaterial(0.85f, 0.83f, 0.78f, 10.0f);

    GLfloat beadRadius = radius;
    GLfloat spacing = beadRadius * 2.2f;  // Slight gap between beads
    int beadCount = (int)(length / spacing);
    if (beadCount < 1) beadCount = 1;

    for (int i = 0; i < beadCount; i++) {
        GLfloat y = topY - i * spacing;
        drawSphere(topX, y, topZ, beadRadius, segments, segments);
    }

    // Larger end knob at the bottom
    GLfloat knobY = topY - length;
    GLfloat knobRadius = beadRadius * 2.5f;
    if (drawKnob) {
        drawSphere(topX, knobY, topZ, knobRadius, segments, segments);
    }
}

void drawElectricalOutlet(GLfloat centerX, GLfloat centerY, GLfloat wallFrontZ) {
    // Slight depth offsets keep the layered pieces from z-fighting.
    GLfloat plateWidth = 0.58f;
    GLfloat plateHeight = 0.90f;
    GLfloat plateDepth = 0.03f;

    GLfloat insetWidth = 0.50f;
    GLfloat insetHeight = 0.82f;
    GLfloat insetDepth = 0.015f;

    GLfloat detailDepth = 0.01f;

    // Outer wall plate.
    glPushMatrix();
        glTranslatef(centerX, centerY, wallFrontZ + plateDepth * 0.5f);
        glScalef(plateWidth, plateHeight, plateDepth);
        setMaterial(0.90f, 0.89f, 0.85f, 30.0f);
        drawCube(1.0f);
    glPopMatrix();

    // Inner raised face.
    glPushMatrix();
        glTranslatef(centerX, centerY, wallFrontZ + plateDepth + insetDepth * 0.5f);
        glScalef(insetWidth, insetHeight, insetDepth);
        setMaterial(0.95f, 0.94f, 0.90f, 20.0f);
        drawCube(1.0f);
    glPopMatrix();

    // Top and bottom screws.
    glPushMatrix();
        glTranslatef(centerX, centerY + 0.32f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.05f, 0.05f, detailDepth);
        setMaterial(0.45f, 0.45f, 0.45f, 60.0f);
        drawCube(1.0f);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(centerX, centerY - 0.32f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.05f, 0.05f, detailDepth);
        setMaterial(0.45f, 0.45f, 0.45f, 60.0f);
        drawCube(1.0f);
    glPopMatrix();

    // Upper receptacle slots.
    glPushMatrix();
        glTranslatef(centerX - 0.08f, centerY + 0.16f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.03f, 0.14f, detailDepth);
        setMaterial(0.08f, 0.08f, 0.08f, 5.0f);
        drawCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(centerX + 0.08f, centerY + 0.16f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.03f, 0.14f, detailDepth);
        setMaterial(0.08f, 0.08f, 0.08f, 5.0f);
        drawCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(centerX, centerY + 0.08f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.07f, 0.05f, detailDepth);
        setMaterial(0.08f, 0.08f, 0.08f, 5.0f);
        drawCube(1.0f);
    glPopMatrix();

    // Lower receptacle slots.
    glPushMatrix();
        glTranslatef(centerX - 0.08f, centerY - 0.16f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.03f, 0.14f, detailDepth);
        setMaterial(0.08f, 0.08f, 0.08f, 5.0f);
        drawCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(centerX + 0.08f, centerY - 0.16f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.03f, 0.14f, detailDepth);
        setMaterial(0.08f, 0.08f, 0.08f, 5.0f);
        drawCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(centerX, centerY - 0.24f, wallFrontZ + plateDepth + insetDepth + detailDepth * 0.5f);
        glScalef(0.07f, 0.05f, detailDepth);
        setMaterial(0.08f, 0.08f, 0.08f, 5.0f);
        drawCube(1.0f);
    glPopMatrix();
}

void drawWindows(int rows, int cols,
                 GLfloat buildingX, GLfloat buildingY, GLfloat buildingZ,
                 GLfloat buildingW, GLfloat buildingH, GLfloat buildingD,
                 GLfloat offsetX, GLfloat offsetY,
                 GLfloat spacingX, GLfloat spacingY,
                 GLfloat winWidth, GLfloat winHeight,
                 WindowStyle* styles = NULL, int styleCount = 0) {
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

    // Default style if none provided: solid blue
    WindowStyle defaultStyle = { 0.3f, 0.5f, 0.8f, 1.0f, 0.3f, 0.5f, 0.8f };
    if (styles == NULL) {
        styles = &defaultStyle;
        styleCount = 1;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, windowTexture);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);

    int windowIndex = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            // Pick style for this window (cycles if fewer styles than windows)
            WindowStyle s = styles[windowIndex % styleCount];
            windowIndex++;

            GLfloat x1 = startX + c * (winWidth + spacingX);
            GLfloat x2 = x1 + winWidth;
            GLfloat y1 = startY + r * (winHeight + spacingY);
            GLfloat y2 = y1 + winHeight;

            // Tile the texture based on window size
            GLfloat texScale = 1.0f;
            GLfloat texU = winWidth / texScale;
            GLfloat texV = winHeight / texScale;

            // Split point in world space and texture space
            GLfloat ySplit = y2 - s.splitRatio * winHeight;
            GLfloat texVSplit = s.splitRatio * texV;

            // Top section
            setMaterial(s.r, s.g, s.b, 80.0f);
            glTexCoord2f(0.0f, texVSplit); glVertex3f(x1, ySplit, frontZ);
            glTexCoord2f(texU, texVSplit); glVertex3f(x2, ySplit, frontZ);
            glTexCoord2f(texU, 0.0f);     glVertex3f(x2, y2, frontZ);
            glTexCoord2f(0.0f, 0.0f);     glVertex3f(x1, y2, frontZ);

            // Bottom section (only if there is a split)
            if (s.splitRatio < 1.0f) {
                setMaterial(s.r2, s.g2, s.b2, 80.0f);
                glTexCoord2f(0.0f, texV);      glVertex3f(x1, y1, frontZ);
                glTexCoord2f(texU, texV);      glVertex3f(x2, y1, frontZ);
                glTexCoord2f(texU, texVSplit); glVertex3f(x2, ySplit, frontZ);
                glTexCoord2f(0.0f, texVSplit); glVertex3f(x1, ySplit, frontZ);
            }
        }
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawScene()
{
    // Draw ground plane first (at Y = -0.5)
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
        //setMaterial(239.0f/255.0f, 220.0f/255.0f, 198.0f/255.0f); // Warm light facade color from reference.
        setMaterial(255/255.0f, 245/255.0f, 227/255.0f);
        drawCube(cubeSize);
    glPopMatrix();

    // ******** Windows ******** //
    // WindowStyle: { r, g, b, splitRatio, r2, g2, b2 }
    //   - r,g,b:       top/main color (0.0-1.0)
    //   - splitRatio:  1.0 = solid color, 0.7 = top 70% / bottom 30%
    //   - r2,g2,b2:    bottom color (only used when splitRatio < 1.0)
    //
    // Pass one style per window, or fewer to cycle through them.
    // Pass NULL for default blue.

    // Row 1 window coloring styles
    WindowStyle row1Styles[] ={
        split(winColor3, 0.1f, winColor2),
        solid(winColor1),
        split(winColor3, 0.1f, winColor1),
        solid(winColor3),
        split(winColor3, 0.6f, winColor1),
        split(winColor3, 0.6f, winColor1),
        solid(winColor3)
    };

    // Row 2 window coloring styles
    WindowStyle row2Styles[] ={
        solid(winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2)
    };

    // Row 3 window coloring styles
    WindowStyle row3Styles[] ={
        solid(winColor4),
        solid(winColor4),
        solid(winColor4),
        solid(winColor4),
        solid(winColor4),
        solid(winColor4),
        solid(winColor4)
    };
    
    // Row 4 window coloring styles
    WindowStyle row4Styles[] ={
        split(winColor4, 0.64f, winColor2),
        split(winColor4, 0.35f, winColor2),
        solid(winColor4),
        split(winColor4, 0.17, winColor2),
        split(winColor4, 0.8f, winColor2),
        split(winColor4, 0.85f, winColor2),
        split(winColor4, 0.90f, winColor2)
    };

    // Row 5 window coloring styles
    WindowStyle row5Styles[] ={
        solid(winColor5),
        solid(winColor5),
        solid(winColor5),
        solid(winColor5),
        solid(winColor5),
        solid(winColor5),
        solid(winColor5)
    };

    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, 2.2f, 0.08f, 0.08f, 2.0f, 2.3,
                row1Styles, 7);

    // Row 2 - short windows
    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, 0.4f, 0.08f, 0.08f, 2.0f, 0.7,
                row2Styles, 7);

    // Row 3 - short windows
    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, -0.4f, 0.08f, 0.08f, 2.0f, 0.7,
                row3Styles, 7);

    // Row 4 - tall windows
    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, -2.0f, 0.08f, 0.08f, 2.0f, 2.3,
                row4Styles, 7);

    // Row 5 (bottom) - tall windows
    drawWindows(1, 7, bx, by, bz, buildingW, buildingH, buildingD,
                -2.7f, -4.6f, 0.08f, 0.08f, 2.0f, 2.3,
                row5Styles, 7);

    // ******** Two Window Frames ******** //
    GLfloat frameWidth = 15.4f;
    GLfloat frameHeight = 14.0f;
    GLfloat frameDepth = 0.12f;
    GLfloat frameBorderThickness = 0.28f;
    GLfloat frameDividerThickness = 0.25f;

    GLfloat frameFrontFaceZ = -6.0f;
    GLfloat horizontalShift = -0.5f; // Shift frame, curtain, and wall together.
    GLfloat leftFrameX = -2.25f + horizontalShift;  // CHANGE FRAME POSITION: move left/right for left frame
    GLfloat leftFrameY = 6.5f;   // CHANGE FRAME POSITION: move up/down for left frame
    drawWindowFrame(leftFrameX, leftFrameY, frameFrontFaceZ,
                    frameWidth, frameHeight, frameDepth,
                    frameBorderThickness, frameDividerThickness);

    GLfloat glassAlpha = 0.5f;
    GLfloat glassForwardOffset = 0.02f;
    drawWindowTextureOverlay(leftFrameX, leftFrameY, frameFrontFaceZ,
                             frameWidth, frameHeight, frameDepth,
                             glassAlpha, glassForwardOffset);

    // ******** Right Curtain Planes ******** //
    // Keep curtain aligned with frame height/depth but offset right so it does not block the frame.
    GLfloat frameRightEdgeX = leftFrameX + frameWidth * 0.5f - 0.25f;
    GLfloat curtainGap = 0.50f;
    GLfloat curtainDepth = 0.03f;
    GLfloat curtainRightExtension = 1.1f; // Make the right curtain a little longer to the right.
    GLfloat curtainCenterY = leftFrameY + 1.0f;
    GLfloat curtainCenterZ = frameFrontFaceZ + frameDepth * 0.5f + 0.05f;
    GLfloat curtainLeftX = frameRightEdgeX + curtainGap;
    GLfloat curtainRightX = curtainCenterZ + 12.0f + horizontalShift + curtainRightExtension;
    if (curtainRightX < curtainLeftX + 0.5f) {
        curtainRightX = curtainLeftX + 0.5f;
    }
    GLfloat curtainWidth = curtainRightX - curtainLeftX;
    GLfloat curtainCenterX = (curtainLeftX + curtainRightX) * 0.5f;

    // Main curtain panel in dark gray, slightly darker than the frame metal color.
    glPushMatrix();
        glTranslatef(curtainCenterX, curtainCenterY, curtainCenterZ);
        glScalef(curtainWidth, frameHeight, curtainDepth);
        setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 30.0f);
        drawCube(1.0f);
    glPopMatrix();

    GLfloat curtainOverlayAlpha = 0.1f;
    GLfloat curtainOverlayForwardOffset = 0.015f;

    // Bottom curtain band (matches the lower section visible in the reference image).
    GLfloat curtainBottomBandHeight = 0.38f;
    GLfloat curtainBottomBandCenterY = leftFrameY - frameHeight * 0.5f + curtainBottomBandHeight * 0.5f;
    glPushMatrix();
        glTranslatef(curtainCenterX, curtainBottomBandCenterY, curtainCenterZ + 0.01f);
        glScalef(curtainWidth, curtainBottomBandHeight, curtainDepth);
        setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 30.0f);
        drawCube(1.0f);
    glPopMatrix();

    // ******** Draw String (blinds pull cord) ******** //

    //right-most string
    GLfloat stringX = curtainLeftX - 0.08f; // Position the string to the left of the right curtain
    GLfloat stringTopY = frameHeight;
    GLfloat stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    GLfloat stringLength = 11.25f;
    GLfloat stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, true);

    // right string with no knob
    stringX = curtainLeftX - 0.08f;
    stringTopY = frameHeight - 11.25f;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 1.25f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, false);
    
    
    // left string
    stringX = curtainLeftX - 0.23f; // Position the string to the left of the right curtain
    stringTopY = frameHeight;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 12.0f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, true);

    // bottom left string with knob
    stringX = curtainLeftX - 0.23f; // Position the string to the left of the right curtain
    stringTopY = frameHeight - 12.0f;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 1.0f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, true);

    // draw hanging string no knob
    stringX = curtainLeftX - 0.23f; // Position the string to the left of the right curtain
    stringTopY = frameHeight - 13.0f;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 1.0f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, false);

    // Map both curtain planes into one shared V range so the texture continues downward.
    GLfloat mainCurtainTopY = curtainCenterY + frameHeight * 0.5f;
    GLfloat mainCurtainBottomY = curtainCenterY - frameHeight * 0.5f;
    GLfloat bottomBandTopY = curtainBottomBandCenterY + curtainBottomBandHeight * 0.5f;
    GLfloat bottomBandBottomY = curtainBottomBandCenterY - curtainBottomBandHeight * 0.5f;
    GLfloat combinedOverlayHeight = mainCurtainTopY - bottomBandBottomY;
    if (combinedOverlayHeight < 0.001f) {
        combinedOverlayHeight = 0.001f;
    }

    GLfloat mainTexVTop = 0.0f;
    GLfloat mainTexVBottom = (mainCurtainTopY - mainCurtainBottomY) / combinedOverlayHeight;
    GLfloat bandTexVTop = (mainCurtainTopY - bottomBandTopY) / combinedOverlayHeight;
    GLfloat bandTexVBottom = 1.0f;

    drawWindowTextureOverlay(curtainCenterX, curtainCenterY,
                             curtainCenterZ - curtainDepth * 0.5f,
                             curtainWidth, frameHeight, curtainDepth,
                             curtainOverlayAlpha, curtainOverlayForwardOffset,
                             mainTexVTop, mainTexVBottom);
    drawWindowTextureOverlay(curtainCenterX, curtainBottomBandCenterY,
                             (curtainCenterZ + 0.01f) - curtainDepth * 0.5f,
                             curtainWidth, curtainBottomBandHeight, curtainDepth,
                             curtainOverlayAlpha, curtainOverlayForwardOffset,
                             bandTexVTop, bandTexVBottom);

    // Bridge the texture through the vertical gap so the pattern is continuous.
    GLfloat gapTopY = mainCurtainBottomY;
    GLfloat gapBottomY = bottomBandTopY;
    GLfloat gapHeight = gapTopY - gapBottomY;
    if (gapHeight > 0.001f) {
        GLfloat gapCenterY = (gapTopY + gapBottomY) * 0.5f;
        drawWindowTextureOverlay(curtainCenterX, gapCenterY,
                                 (curtainCenterZ + 0.005f) - curtainDepth * 0.5f,
                                 curtainWidth, gapHeight, curtainDepth,
                                 curtainOverlayAlpha, curtainOverlayForwardOffset,
                                 mainTexVBottom, bandTexVTop);
    }

    // Bottom wall section under the frame (same thickness/depth as frame).
    GLfloat wallSectionTopY = leftFrameY - frameHeight * 0.5f;
    GLfloat wallSectionBottomY = by - buildingH - 3.0f;
    GLfloat wallSectionHeight = wallSectionTopY - wallSectionBottomY;

    // CHANGE WALL WIDTH: adjust these two values to span the full projection window bottom.
    GLfloat wallSectionLeftX = bx - buildingW + 0.25f + horizontalShift;
    GLfloat wallSectionRightX = bx + buildingW - 0.25f + horizontalShift;
    GLfloat wallSectionWidth = wallSectionRightX - wallSectionLeftX;
    GLfloat wallSectionCenterX = (wallSectionLeftX + wallSectionRightX) * 0.5f;

    GLfloat wallSectionCenterY = wallSectionBottomY + wallSectionHeight * 0.5f;
    GLfloat wallSectionCenterZ = (frameFrontFaceZ - 0.01f) + frameDepth * 0.5f;

    glPushMatrix();
        glTranslatef(wallSectionCenterX, wallSectionCenterY, wallSectionCenterZ);
        glScalef(wallSectionWidth, wallSectionHeight, frameDepth); // Same depth as frame thickness.
        setMaterial(225.0f/255.0f, 184.0f/255.0f, 142.0f/255.0f); // Warm orange-beige wall color from reference.
        drawCube(1.0f);
    glPopMatrix();

    // Wall outlet positioned low on the wall like the reference image.
    GLfloat wallFrontZ = wallSectionCenterZ + frameDepth * 0.5f;
    GLfloat outletX = wallSectionCenterX + wallSectionWidth * 0.15f;
    GLfloat outletY = wallSectionBottomY + wallSectionHeight * 0.5f;
    drawElectricalOutlet(outletX, outletY, wallFrontZ);

    // ******** Building Roof ******** //
    bx = 0.0f, by = 9.75f, bz = -10.0f; // Position on top of building
    sx = 10.0f, sy = 0.25f, sz = 4.0f;   // Scale
    cubeSize = 2.0f;

    // Draw the roof cube
    glPushMatrix();
        glTranslatef(bx, by + halfCube * sy, bz); // Position on top of building
        glScalef(sx, sy, sz);
        //setMaterial(205.0f/255.0f, 170.0f/255.0f, 134.0f/255.0f); // Warm tan roof color from reference.
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
        //setMaterial(108.0f/255.0f, 85.0f/255.0f, 62.0f/255.0f); // Dark warm roof trim color from reference.
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
    //drawCameraCoordinatesOverlay();
    glutSwapBuffers();
}

void reshape(int width, int height) 
{
    if (height == 0) height = 1; // Prevent division by zero
    windowWidth = width;
    windowHeight = height;
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
            cameraX = 1.5f;
            cameraY = 5.0f;
            cameraZ = 11.45f;
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
    glutInitWindowSize(625, 738);
    glutCreateWindow("3D Scene with Camera and Lighting");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}

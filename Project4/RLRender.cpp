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


// GLfloat cameraX = -1.0f;
// GLfloat cameraY = 3.75f;
// GLfloat cameraZ = 15.95f;

GLfloat cameraX = 4.0f;
GLfloat cameraY = 1.75f;
GLfloat cameraZ = -0.55f;


GLfloat cameraAngleX = 0.0f;  // Pitch (up/down rotation)
GLfloat cameraAngleY = 0.0f;  // Yaw (left/right rotation)

const GLfloat MOVE_SPEED = 0.5f;    // Units per key press
const GLfloat ROTATE_SPEED = 2.0f;  // Degrees per key press

GLuint windowTexture;
GLuint carpetTexture;
int windowWidth = 800;
int windowHeight = 600;
bool showCoordinateSystemOverlay = true;

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

void drawCoordinateSystemOverlay() {
    const GLfloat axisLength = 55.0f;
    const GLfloat margin = 16.0f;
    const GLfloat originX = static_cast<GLfloat>(windowWidth) - margin - axisLength;
    const GLfloat originY = static_cast<GLfloat>(windowHeight) - margin - axisLength;
    const GLfloat zAxisLength = axisLength * 0.45f;
    const GLfloat infoX = static_cast<GLfloat>(windowWidth) - 150.0f;
    const GLfloat infoTopY = static_cast<GLfloat>(windowHeight) - 20.0f;

    char xText[32];
    char yText[32];
    char zText[32];
    std::snprintf(xText, sizeof(xText), "X: %.2f", cameraX);
    std::snprintf(yText, sizeof(yText), "Y: %.2f", cameraY);
    std::snprintf(zText, sizeof(zText), "Z: %.2f", cameraZ);

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
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

    glLineWidth(2.0f);
    glBegin(GL_LINES);
        // X axis (red)
        glColor3f(0.95f, 0.20f, 0.20f);
        glVertex2f(originX, originY);
        glVertex2f(originX + axisLength, originY);

        // Y axis (green)
        glColor3f(0.20f, 0.85f, 0.20f);
        glVertex2f(originX, originY);
        glVertex2f(originX, originY + axisLength);

        // Z axis (blue)
        glColor3f(0.20f, 0.45f, 0.95f);
        glVertex2f(originX, originY);
        glVertex2f(originX - zAxisLength, originY - zAxisLength);
    glEnd();

    glColor3f(0.95f, 0.20f, 0.20f);
    drawBitmapText("X", originX + axisLength + 6.0f, originY - 4.0f, GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.20f, 0.85f, 0.20f);
    drawBitmapText("Y", originX - 4.0f, originY + axisLength + 8.0f, GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.20f, 0.45f, 0.95f);
    drawBitmapText("Z", originX - zAxisLength - 12.0f, originY - zAxisLength - 4.0f, GLUT_BITMAP_HELVETICA_12);

    // Numeric camera coordinates in top-right corner.
    glColor3f(0.95f, 0.20f, 0.20f);
    drawBitmapText(xText, infoX, infoTopY, GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.20f, 0.85f, 0.20f);
    drawBitmapText(yText, infoX, infoTopY - 16.0f, GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.20f, 0.45f, 0.95f);
    drawBitmapText(zText, infoX, infoTopY - 32.0f, GLUT_BITMAP_HELVETICA_12);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}


void drawGroundPlane(GLfloat minX, GLfloat maxX, GLfloat y, GLfloat minZ, GLfloat maxZ)
{
    int divisions = 200;
    GLfloat stepX = (maxX - minX) / divisions;
    GLfloat stepZ = (maxZ - minZ) / divisions;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            GLfloat x = minX + i * stepX;
            GLfloat z = minZ + j * stepZ;

            if ((i + j) % 2 == 0) {
                setMaterial(0.4f, 0.4f, 0.4f, 10.0f);  // Light gray
            } else {
                setMaterial(0.2f, 0.2f, 0.2f, 10.0f);  // Dark gray
            }

            glVertex3f(x,         y, z);
            glVertex3f(x + stepX, y, z);
            glVertex3f(x + stepX, y, z + stepZ);
            glVertex3f(x,         y, z + stepZ);
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
                     GLfloat dividerThickness,
                     bool includeMiddleSection = true,
                     bool drawLeftBorder = true,
                     bool drawRightBorder = true) {
    GLfloat halfW = frameWidth * 0.5f;
    GLfloat halfH = frameHeight * 0.5f;
    GLfloat centerZ = frontFaceZ + frameDepth * 0.5f;
    GLfloat innerHeight = frameHeight - (2.0f * borderThickness);

    if (innerHeight < borderThickness) {
        innerHeight = borderThickness;
    }

    // Frame color similar to the metal/aluminum look in your reference image.
    setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 30.0f);

    if (drawLeftBorder) {
        // Left vertical bar
        glPushMatrix();
            glTranslatef(centerX - halfW + borderThickness * 0.5f, centerY, centerZ);
            glScalef(borderThickness, frameHeight, frameDepth);
            drawCube(1.0f);
        glPopMatrix();
    }

    if (drawRightBorder) {
        // Right vertical bar
        glPushMatrix();
            glTranslatef(centerX + halfW - borderThickness * 0.5f, centerY, centerZ);
            glScalef(borderThickness, frameHeight, frameDepth);
            drawCube(1.0f);
        glPopMatrix();
    }

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

    if (includeMiddleSection) {
        // Center divider (gives the two-panel frame look)
        glPushMatrix();
            glTranslatef(centerX, centerY, centerZ);
            glScalef(dividerThickness, innerHeight, frameDepth);
            drawCube(1.0f);
        glPopMatrix();
    }
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

void drawCurtainSegment(GLfloat leftX, GLfloat width,
                        GLfloat topY, GLfloat height,
                        GLfloat centerZ, GLfloat depth,
                        GLfloat bottomBandHeight,
                        GLfloat minBandBottomY,
                        GLfloat bandBottomY,
                        GLfloat overlayAlpha,
                        GLfloat overlayForwardOffset) {
    if (width <= 0.001f || height <= 0.001f) {
        return;
    }

    GLfloat centerX = leftX + width * 0.5f;
    GLfloat centerY = topY - height * 0.5f; // Top-aligned curtains: varying heights drop downward.

    // Main curtain panel in dark gray, slightly darker than the frame metal color.
    glPushMatrix();
        glTranslatef(centerX, centerY, centerZ);
        glScalef(width, height, depth);
        setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 30.0f);
        drawCube(1.0f);
    glPopMatrix();

    GLfloat bandHeight = bottomBandHeight;
    if (bandHeight > height) {
        bandHeight = height;
    }
    if (bandHeight < 0.001f) {
        bandHeight = 0.001f;
    }

    // Left-side bands can vary slightly, but never below the frame-bottom baseline.
    GLfloat clampedBandBottomY = bandBottomY;
    if (clampedBandBottomY < minBandBottomY) {
        clampedBandBottomY = minBandBottomY;
    }
    // This intentionally leaves a gap between the main curtain and the lower band.
    GLfloat bandCenterY = clampedBandBottomY + bandHeight * 0.5f;
    glPushMatrix();
        glTranslatef(centerX, bandCenterY, centerZ + 0.01f);
        glScalef(width, bandHeight, depth);
        setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 30.0f);
        drawCube(1.0f);
    glPopMatrix();

    // Map both curtain planes into one shared V range so the texture continues downward.
    GLfloat mainTopY = centerY + height * 0.5f;
    GLfloat mainBottomY = centerY - height * 0.5f;
    GLfloat bandTopY = bandCenterY + bandHeight * 0.5f;
    GLfloat bandBottomEdgeY = bandCenterY - bandHeight * 0.5f;
    GLfloat combinedOverlayHeight = mainTopY - bandBottomEdgeY;
    if (combinedOverlayHeight < 0.001f) {
        combinedOverlayHeight = 0.001f;
    }

    GLfloat mainTexVTop = 0.0f;
    GLfloat mainTexVBottom = (mainTopY - mainBottomY) / combinedOverlayHeight;
    GLfloat bandTexVTop = (mainTopY - bandTopY) / combinedOverlayHeight;
    GLfloat bandTexVBottom = 1.0f;

    drawWindowTextureOverlay(centerX, centerY,
                             centerZ - depth * 0.5f,
                             width, height, depth,
                             overlayAlpha, overlayForwardOffset,
                             mainTexVTop, mainTexVBottom);
    drawWindowTextureOverlay(centerX, bandCenterY,
                             (centerZ + 0.01f) - depth * 0.5f,
                             width, bandHeight, depth,
                             overlayAlpha, overlayForwardOffset,
                             bandTexVTop, bandTexVBottom);

    // Bridge the texture through the vertical gap so the pattern is continuous.
    GLfloat gapTopY = mainBottomY;
    GLfloat gapBottomY = bandTopY;
    GLfloat gapHeight = gapTopY - gapBottomY;
    if (gapHeight > 0.001f) {
        GLfloat gapCenterY = (gapTopY + gapBottomY) * 0.5f;
        drawWindowTextureOverlay(centerX, gapCenterY,
                                 (centerZ + 0.005f) - depth * 0.5f,
                                 width, gapHeight, depth,
                                 overlayAlpha, overlayForwardOffset,
                                 mainTexVBottom, bandTexVTop);
    }
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

void drawBuilding(GLfloat posX, GLfloat posY, GLfloat posZ)
{
    GLfloat sx = 12.0f, sy = 6.5f, sz = 1.0f;
    GLfloat cubeSize = 2.0f;
    GLfloat halfCube = cubeSize / 2.0f;

    GLfloat buildingW = halfCube * sx;
    GLfloat buildingH = halfCube * sy;
    GLfloat buildingD = halfCube * sz;

    // ******** Building ******** //
    glPushMatrix();
        glTranslatef(posX, posY, posZ);
        glScalef(sx, sy, sz);
        setMaterial(255/255.0f, 245/255.0f, 227/255.0f);
        drawCube(cubeSize);
    glPopMatrix();

    // ******** Windows ******** //
    WindowStyle row1Styles[] = {
        split(winColor3, 0.1f, winColor2),
        solid(winColor1),
        split(winColor3, 0.1f, winColor1),
        solid(winColor3),
        split(winColor3, 0.6f, winColor1),
        split(winColor3, 0.6f, winColor1),
        solid(winColor3)
    };
    WindowStyle row2Styles[] = {
        solid(winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2),
        split(winColor1, 0.5f, winColor2)
    };
    WindowStyle row3Styles[] = {
        solid(winColor4), solid(winColor4), solid(winColor4),
        solid(winColor4), solid(winColor4), solid(winColor4), solid(winColor4)
    };
    WindowStyle row4Styles[] = {
        split(winColor4, 0.64f, winColor2),
        split(winColor4, 0.35f, winColor2),
        solid(winColor4),
        split(winColor4, 0.17f, winColor2),
        split(winColor4, 0.8f,  winColor2),
        split(winColor4, 0.85f, winColor2),
        split(winColor4, 0.90f, winColor2)
    };
    WindowStyle row5Styles[] = {
        solid(winColor5), solid(winColor5), solid(winColor5),
        solid(winColor5), solid(winColor5), solid(winColor5), solid(winColor5)
    };

    drawWindows(1, 7, posX, posY, posZ, buildingW, buildingH, buildingD,
                -2.7f, 2.2f,  0.08f, 0.08f, 2.0f, 2.3f, row1Styles, 7);
    drawWindows(1, 7, posX, posY, posZ, buildingW, buildingH, buildingD,
                -2.7f, 0.4f,  0.08f, 0.08f, 2.0f, 0.7f, row2Styles, 7);
    drawWindows(1, 7, posX, posY, posZ, buildingW, buildingH, buildingD,
                -2.7f, -0.4f, 0.08f, 0.08f, 2.0f, 0.7f, row3Styles, 7);
    drawWindows(1, 7, posX, posY, posZ, buildingW, buildingH, buildingD,
                -2.7f, -2.0f, 0.08f, 0.08f, 2.0f, 2.3f, row4Styles, 7);
    drawWindows(1, 7, posX, posY, posZ, buildingW, buildingH, buildingD,
                -2.7f, -4.6f, 0.08f, 0.08f, 2.0f, 2.3f, row5Styles, 7);

    // ******** Roofs ******** //
    GLfloat buildingTopY = posY + buildingH;

    // First roof layer (flush with building top)
    glPushMatrix();
        glTranslatef(posX, buildingTopY + halfCube * 0.15f, posZ);
        glScalef(12.0f, 0.15f, 4.0f);
        setMaterial(255/255.0f, 245/255.0f, 227/255.0f);
        drawCube(cubeSize);
    glPopMatrix();

    // Second roof layer (trim band on top)
    glPushMatrix();
        glTranslatef(posX, buildingTopY + 0.25f + halfCube * 0.15f, posZ);
        glScalef(12.0f, 0.15f, 4.0f);
        setMaterial(65/255.0f, 65/255.0f, 65/255.0f);
        drawCube(cubeSize);
    glPopMatrix();
}

void drawScene()
{
    // Ground plane drawn later, after shell bounds are computed.

    drawBuilding(0.0f, 3.25f, -10.0f);

    drawBuilding(16.0f, 3.25f, -10.0f);

    // Room interior is anchored to the first building at (0, 3.25, -10).
    const GLfloat by = 3.25f;
    const GLfloat buildingH = 6.5f;  // halfCube(1) * sy(6.5)

    // ******** Window Frame Row ******** //
    GLfloat frameWidth = 15.4f;
    GLfloat frameHeight = 14.0f;
    GLfloat frameDepth = 0.12f;
    GLfloat frameBorderThickness = 0.28f;
    GLfloat frameDividerThickness = 0.25f;

    GLfloat frameFrontFaceZ = -6.0f;
    GLfloat horizontalShift = -0.5f; // Shift frame, curtain, and wall together.
    GLfloat originalFrameCenterX = -2.25f + horizontalShift;
    GLfloat frameCenterY = 6.5f;

    // Left->right frame sequence: three standard frames on the left of original,
    // one standard on the right, and the far-right half-width frame (no middle).
    GLfloat frameWidths[] = {
        frameWidth, frameWidth, frameWidth, frameWidth, frameWidth, frameWidth * 0.5f
    };
    bool frameHasMiddle[] = { true, true, true, true, true, false };
    const int frameCount = static_cast<int>(sizeof(frameWidths) / sizeof(frameWidths[0]));
    const int originalFrameIndex = 3;

    GLfloat frameLeftEdges[6];
    GLfloat frameRightEdges[6];
    GLfloat frameCenters[6];

    // Keep the original frame fixed, then place neighbors with shared-edge math:
    // leftX[i+1] = leftX[i] + width[i] (no spacing between adjacent frames).
    frameLeftEdges[originalFrameIndex] = originalFrameCenterX - frameWidths[originalFrameIndex] * 0.5f;
    frameCenters[originalFrameIndex] = originalFrameCenterX;

    for (int i = originalFrameIndex - 1; i >= 0; --i) {
        frameLeftEdges[i] = frameLeftEdges[i + 1] - frameWidths[i];
        frameCenters[i] = frameLeftEdges[i] + frameWidths[i] * 0.5f;
    }
    for (int i = originalFrameIndex + 1; i < frameCount; ++i) {
        frameLeftEdges[i] = frameLeftEdges[i - 1] + frameWidths[i - 1];
        frameCenters[i] = frameLeftEdges[i] + frameWidths[i] * 0.5f;
    }
    for (int i = 0; i < frameCount; ++i) {
        frameRightEdges[i] = frameLeftEdges[i] + frameWidths[i];
    }

    GLfloat frameRowLeftEdgeX = frameLeftEdges[0];
    GLfloat frameRowRightEdgeX = frameRightEdges[frameCount - 1];
    GLfloat frameRowWidth = frameRowRightEdgeX - frameRowLeftEdgeX; // 5.5 * standard width

    GLfloat glassAlpha = 0.5f;
    GLfloat glassForwardOffset = 0.02f;
    for (int i = 0; i < frameCount; ++i) {
        // Seam de-duplication (Option B): shared boundaries are emitted once via right borders.
        bool drawLeftBorder = (i == 0);
        bool drawRightBorder = true;

        // The far-right frame is half-width and omits the middle divider.
        drawWindowFrame(frameCenters[i], frameCenterY, frameFrontFaceZ,
                        frameWidths[i], frameHeight, frameDepth,
                        frameBorderThickness, frameDividerThickness,
                        frameHasMiddle[i],
                        drawLeftBorder, drawRightBorder);
        drawWindowTextureOverlay(frameCenters[i], frameCenterY, frameFrontFaceZ,
                                 frameWidths[i], frameHeight, frameDepth,
                                 glassAlpha, glassForwardOffset);
    }

    // ******** Curtain Segments ******** //
    // No curtain is placed on the camera-facing middle frame (originalFrameIndex).
    const int leftNearCurtainFrameIndex = originalFrameIndex - 1;
    const int leftMidCurtainFrameIndex = originalFrameIndex - 2;
    const int leftFarCurtainFrameIndex = originalFrameIndex - 3;
    const int rightMainCurtainFrameIndex = frameCount - 2;
    const int rightFarCurtainFrameIndex = frameCount - 1;

    GLfloat leftNearCurtainWidth = frameWidths[leftNearCurtainFrameIndex];
    GLfloat leftMidCurtainWidth = frameWidths[leftMidCurtainFrameIndex];
    GLfloat leftFarCurtainWidth = frameWidths[leftFarCurtainFrameIndex];
    GLfloat rightMainCurtainWidth = frameWidths[rightMainCurtainFrameIndex];
    GLfloat rightFarCurtainWidth = frameWidths[rightFarCurtainFrameIndex];

    GLfloat curtainDepth = 0.03f;
    GLfloat curtainCenterZ = frameFrontFaceZ + frameDepth * 0.5f + 0.05f;
    GLfloat curtainOverlayAlpha = 0.1f;
    GLfloat curtainOverlayForwardOffset = 0.015f;
    GLfloat baseBottomBandHeight = 0.38f;
    GLfloat frameTopY = frameCenterY + frameHeight * 0.5f;
    GLfloat curtainTopY = frameTopY;
    GLfloat frameBottomY = frameCenterY - frameHeight * 0.5f;
    // Keep curtains no higher than frame top; trim heights so they do not shift downward.
    GLfloat curtainHeightTrim = 1.0f;
    GLfloat rightMainCurtainHeight = frameHeight - curtainHeightTrim;
    GLfloat rightFarCurtainHeight = (frameHeight * 0.85f) - curtainHeightTrim;
    GLfloat leftNearCurtainHeight = (frameHeight * 0.45f) - curtainHeightTrim;
    GLfloat leftMidCurtainHeight = (frameHeight * 0.60f) - curtainHeightTrim;
    GLfloat leftFarCurtainHeight = (frameHeight * 0.52f) - curtainHeightTrim;

    GLfloat leftNearBottomBandHeight = baseBottomBandHeight;
    GLfloat leftMidBottomBandHeight = baseBottomBandHeight * 0.92f;
    GLfloat leftFarBottomBandHeight = baseBottomBandHeight * 1.08f;
    GLfloat rightMainBottomBandHeight = baseBottomBandHeight;
    GLfloat rightFarBottomBandHeight = baseBottomBandHeight;

    // Left-side bottom bands vary slightly upward, never below frameBottomY.
    GLfloat leftNearBandBottomY = frameBottomY + 2.02f;
    GLfloat leftMidBandBottomY = frameBottomY;
    GLfloat leftFarBandBottomY = frameBottomY + 1.14f;
    GLfloat rightMainBandBottomY = frameBottomY;
    GLfloat rightFarBandBottomY = frameBottomY;

    GLfloat leftNearCurtainLeftX = frameLeftEdges[leftNearCurtainFrameIndex];
    GLfloat leftMidCurtainLeftX = frameLeftEdges[leftMidCurtainFrameIndex];
    GLfloat leftFarCurtainLeftX = frameLeftEdges[leftFarCurtainFrameIndex];
    GLfloat rightMainCurtainLeftX = frameLeftEdges[rightMainCurtainFrameIndex];
    GLfloat rightFarCurtainLeftX = frameLeftEdges[rightFarCurtainFrameIndex];

    // Each curtain spans exactly the width of the frame it covers, with varied heights.
    drawCurtainSegment(leftFarCurtainLeftX, leftFarCurtainWidth,
                       curtainTopY, leftFarCurtainHeight,
                       curtainCenterZ, curtainDepth,
                       leftFarBottomBandHeight, frameBottomY, leftFarBandBottomY,
                       curtainOverlayAlpha, curtainOverlayForwardOffset);
    drawCurtainSegment(leftMidCurtainLeftX, leftMidCurtainWidth,
                       curtainTopY, leftMidCurtainHeight,
                       curtainCenterZ, curtainDepth,
                       leftMidBottomBandHeight, frameBottomY, leftMidBandBottomY,
                       curtainOverlayAlpha, curtainOverlayForwardOffset);
    drawCurtainSegment(leftNearCurtainLeftX, leftNearCurtainWidth,
                       curtainTopY, leftNearCurtainHeight,
                       curtainCenterZ, curtainDepth,
                       leftNearBottomBandHeight, frameBottomY, leftNearBandBottomY,
                       curtainOverlayAlpha, curtainOverlayForwardOffset);
    drawCurtainSegment(rightMainCurtainLeftX, rightMainCurtainWidth,
                       curtainTopY, rightMainCurtainHeight,
                       curtainCenterZ, curtainDepth,
                       rightMainBottomBandHeight, frameBottomY, rightMainBandBottomY,
                       curtainOverlayAlpha, curtainOverlayForwardOffset);
    drawCurtainSegment(rightFarCurtainLeftX, rightFarCurtainWidth,
                       curtainTopY, rightFarCurtainHeight,
                       curtainCenterZ, curtainDepth,
                       rightFarBottomBandHeight, frameBottomY, rightFarBandBottomY,
                       curtainOverlayAlpha, curtainOverlayForwardOffset);

    // ******** Draw String (blinds pull cord) ******** //

    // Keep pull cords with the right main curtain segment.
    GLfloat stringX = rightMainCurtainLeftX - 0.08f;
    GLfloat chainTopY = frameTopY;
    GLfloat chainLengthTrim = frameHeight - frameTopY; // Trim amount to keep chain bottoms from dropping.
    GLfloat stringTopY = chainTopY;
    GLfloat stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    GLfloat stringLength = 11.25f - chainLengthTrim;
    GLfloat stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, true);

    // right string with no knob
    stringX = rightMainCurtainLeftX - 0.08f;
    stringTopY = frameHeight - 11.25f;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 1.25f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, false);
    
    
    stringX = rightMainCurtainLeftX - 0.23f;
    stringTopY = chainTopY;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 12.0f - chainLengthTrim;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, true);

    stringX = rightMainCurtainLeftX - 0.23f;
    stringTopY = frameHeight - 12.0f;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 1.0f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, true);

    stringX = rightMainCurtainLeftX - 0.23f;
    stringTopY = frameHeight - 13.0f;
    stringZ = frameFrontFaceZ + frameDepth + 0.05f;  // Just in front of the glass
    stringLength = 1.0f;
    stringRadius = 0.03f;
    drawDrawString(stringX, stringTopY, stringZ,
                   stringLength, stringRadius, 8, 12, false);

    // Bottom wall section under the frame (same thickness/depth as frame).
    GLfloat wallSectionTopY = frameCenterY - frameHeight * 0.5f;
    GLfloat wallSectionBottomY = by - buildingH - 3.0f;
    GLfloat wallSectionHeight = wallSectionTopY - wallSectionBottomY;

    // Match the wall segment to the exact frame-row span (shared-edge layout).
    GLfloat wallSectionLeftX = frameRowLeftEdgeX;
    GLfloat wallSectionRightX = frameRowRightEdgeX;
    GLfloat wallSectionWidth = frameRowWidth;
    GLfloat wallSectionCenterX = (wallSectionLeftX + wallSectionRightX) * 0.5f;

    GLfloat wallSectionCenterY = wallSectionBottomY + wallSectionHeight * 0.5f;
    GLfloat wallSectionCenterZ = (frameFrontFaceZ - 0.01f) + frameDepth * 0.5f;

    glPushMatrix();
        glTranslatef(wallSectionCenterX, wallSectionCenterY, wallSectionCenterZ);
        glScalef(wallSectionWidth, wallSectionHeight, frameDepth); // Same depth as frame thickness.
        setMaterial(225.0f/255.0f, 184.0f/255.0f, 142.0f/255.0f); // Warm orange-beige wall color from reference.
        drawCube(1.0f);
    glPopMatrix();

    // Rubber baseboard at the bottom of the lower wall: curtain color, full wall width,
    // and slightly protruding forward from the wall face.
    GLfloat baseboardHeight = 0.70f;
    GLfloat baseboardProtrude = 0.03f;
    GLfloat baseboardDepth = frameDepth + baseboardProtrude;
    GLfloat baseboardCenterY = wallSectionBottomY + baseboardHeight * 0.5f;
    GLfloat baseboardCenterZ = wallSectionCenterZ + baseboardProtrude * 0.5f;

    glPushMatrix();
        glTranslatef(wallSectionCenterX, baseboardCenterY, baseboardCenterZ);
        glScalef(wallSectionWidth, baseboardHeight, baseboardDepth);
        setMaterial(90.0f/255.0f, 94.0f/255.0f, 98.0f/255.0f, 20.0f);
        drawCube(1.0f);
    glPopMatrix();

    // Surrounding shell: two side walls, one back wall, floor, and ceiling.
    // Side-wall span uses half the lower wall width; back wall uses full lower wall width.
    GLfloat shellColorR = 225.0f / 255.0f;
    GLfloat shellColorG = 184.0f / 255.0f;
    GLfloat shellColorB = 142.0f / 255.0f;
    GLfloat shellThickness = frameDepth;
    // Double the previous room depth so the shell extends farther back.
    GLfloat sideWallSpan = wallSectionWidth;
    GLfloat backWallWidth = wallSectionWidth;
    GLfloat shellBottomY = wallSectionBottomY;
    GLfloat shellTopY = wallSectionTopY + frameHeight; // Lower wall + frame height.
    GLfloat shellHeight = shellTopY - shellBottomY;
    GLfloat shellCenterY = shellBottomY + shellHeight * 0.5f;

    setMaterial(shellColorR, shellColorG, shellColorB);

    // Ground plane: checkerboard fills the interior floor of the surrounding walls.
    {
        GLfloat floorY    = shellBottomY + shellThickness;
        GLfloat floorMinX = wallSectionLeftX  + shellThickness;
        GLfloat floorMaxX = wallSectionRightX - shellThickness;
        GLfloat floorMinZ = wallSectionCenterZ;
        GLfloat floorMaxZ = wallSectionCenterZ + sideWallSpan - shellThickness;
        drawGroundPlane(floorMinX*.001, floorMaxX*.001, floorY, floorMinZ, floorMaxZ);
    }

    // Left side wall
    glPushMatrix();
        glTranslatef(wallSectionLeftX + shellThickness * 0.5f,
                     shellCenterY,
                     wallSectionCenterZ + sideWallSpan * 0.5f);
        glScalef(shellThickness, shellHeight, sideWallSpan);
        drawCube(1.0f);
    glPopMatrix();

    // Right side wall
    glPushMatrix();
        glTranslatef(wallSectionRightX - shellThickness * 0.5f,
                     shellCenterY,
                     wallSectionCenterZ + sideWallSpan * 0.5f);
        glScalef(shellThickness, shellHeight, sideWallSpan);
        drawCube(1.0f);
    glPopMatrix();

    // Back wall
    glPushMatrix();
        glTranslatef(wallSectionCenterX,
                     shellCenterY,
                     wallSectionCenterZ + sideWallSpan);
        glScalef(backWallWidth, shellHeight, shellThickness);
        drawCube(1.0f);
    glPopMatrix();

    // Floor
    glPushMatrix();
        glTranslatef(wallSectionCenterX,
                     shellBottomY + shellThickness * 0.5f,
                     wallSectionCenterZ + sideWallSpan * 0.5f);
        glScalef(backWallWidth, shellThickness, sideWallSpan);
        drawCube(1.0f);
    glPopMatrix();

    // Carpet texture on the floor top, tiled so it repeats instead of stretching.
    if (carpetTexture != 0) {
        GLfloat floorTopY = shellBottomY + shellThickness + 0.002f;
        GLfloat floorLeftX = wallSectionCenterX - backWallWidth * 0.5f;
        GLfloat floorRightX = wallSectionCenterX + backWallWidth * 0.5f;
        GLfloat floorNearZ = wallSectionCenterZ;
        GLfloat floorFarZ = wallSectionCenterZ + sideWallSpan;

        GLfloat carpetTileWorldSize = 1.2f;
        GLfloat tileU = backWallWidth / carpetTileWorldSize;
        GLfloat tileV = sideWallSpan / carpetTileWorldSize;

        glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, carpetTexture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        setMaterial(0.68f, 0.68f, 0.68f, 8.0f);
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f);  glVertex3f(floorLeftX,  floorTopY, floorNearZ);
            glTexCoord2f(tileU, 0.0f); glVertex3f(floorRightX, floorTopY, floorNearZ);
            glTexCoord2f(tileU, tileV);glVertex3f(floorRightX, floorTopY, floorFarZ);
            glTexCoord2f(0.0f, tileV); glVertex3f(floorLeftX,  floorTopY, floorFarZ);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
    }

    // Ceiling
    glPushMatrix();
        glTranslatef(wallSectionCenterX,
                     shellTopY - shellThickness * 0.5f,
                     wallSectionCenterZ + sideWallSpan * 0.5f);
        glScalef(backWallWidth, shellThickness, sideWallSpan);
        drawCube(1.0f);
    glPopMatrix();

    // Wall outlet positioned low on the wall like the reference image.
    GLfloat wallFrontZ = wallSectionCenterZ + frameDepth * 0.5f;
    // Place socket so its outer-left edge is right of the midpoint of the center frame's right half.
    GLfloat outletPlateWidth = 0.85f; // Matches drawElectricalOutlet() outer plate width.
    GLfloat centerFrameRightHalfMidX = originalFrameCenterX + frameWidths[originalFrameIndex] * 0.25f;
    GLfloat outletLeftEdgeX = centerFrameRightHalfMidX + 0.10f;
    GLfloat outletX = outletLeftEdgeX + outletPlateWidth * 0.5f;
    GLfloat outletY = wallSectionBottomY + wallSectionHeight * 0.5f;
    drawElectricalOutlet(outletX, outletY, wallFrontZ);

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
    if (showCoordinateSystemOverlay) {
        drawCoordinateSystemOverlay();
    }
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
            cameraX = 4.0f;
            cameraY = 1.75f;
            cameraZ = -0.55f;
            cameraAngleX = 0.0f;
            cameraAngleY = 0.0f;
            break;
        case 'c':
        case 'C':
            showCoordinateSystemOverlay = !showCoordinateSystemOverlay;
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

GLuint createCarpetTexture() {
    const int texSize = 64;
    unsigned char pixels[texSize * texSize * 3];

    // Procedural dark carpet: non-checkered, subtle deterministic grain.
    for (int y = 0; y < texSize; ++y) {
        for (int x = 0; x < texSize; ++x) {
            int idx = (y * texSize + x) * 3;
            int grain = ((x * 37 + y * 91 + (x * y) * 11) % 17) - 8;
            int wave = ((x * 3 + y * 5) % 9) - 4;
            int base = 122;
            int shade = base + grain + wave;
            if (shade < 0) shade = 0;
            if (shade > 255) shade = 255;
            pixels[idx + 0] = static_cast<unsigned char>(shade);
            pixels[idx + 1] = static_cast<unsigned char>(shade);
            pixels[idx + 2] = static_cast<unsigned char>(shade);
        }
    }

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
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
    carpetTexture = createCarpetTexture();
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

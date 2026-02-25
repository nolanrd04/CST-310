#ifdef _WIN32
#include <windows.h> // Must be included before GL headers on Windows.
#endif

#include <GL/freeglut.h>
#include <algorithm> // std::min

// -----------------------------------------------------------------------------
// Fixed-function OpenGL demo:
// 8 blue cubes in a 2x4 grid, each with a different material shininess.
// Top row:    2, 4, 8, 16
// Bottom row: 32, 64, 128, 256 (clamped to 128 in OpenGL state; label stays 256)
// -----------------------------------------------------------------------------

// Window dimensions (updated by reshape callback).
static int gWindowWidth  = 1200;
static int gWindowHeight = 700;

// Grid and cube layout.
static const int   kRows       = 2;
static const int   kCols       = 4;
static const float kCubeSize   = 1.35f; // Uniform scale for each unit cube.
static const float kColSpacing = 3.10f; // Horizontal spacing between cube centers.
static const float kRowSpacing = 3.30f; // Vertical spacing between row centers.

// Shininess labels shown under cubes (exact values requested by prompt).
static const int kShininessLabels[kRows][kCols] = {
    { 2,  4,   8,   16 },
    { 32, 64, 128, 256 }
};

static const char* kLabelText[kRows][kCols] = {
    { "2", "4", "8", "16" },
    { "32", "64", "128", "256" }
};

// Computes world-space X center for a column index (0..3).
static float CubeCenterX(int col) {
    const float centerOffset = (kCols - 1) * 0.5f;
    return (static_cast<float>(col) - centerOffset) * kColSpacing;
}

// Computes world-space Y center for a row index (0 = top, 1 = bottom).
static float CubeCenterY(int row) {
    const float centerOffset = (kRows - 1) * 0.5f;
    return (centerOffset - static_cast<float>(row)) * kRowSpacing;
}

// Returns bitmap text width in pixels for centering labels.
static int BitmapStringWidth(void* font, const char* text) {
    int width = 0;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        width += glutBitmapWidth(font, *p);
    }
    return width;
}

// Draws bitmap text at 2D screen coordinates (current projection/modelview expected to be 2D-friendly).
static void DrawBitmapString2D(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        glutBitmapCharacter(font, *p);
    }
}

// Draws a unit cube centered at origin with correct per-face normals.
// Immediate mode is used intentionally (legacy fixed-function request).
static void DrawUnitCube() {
    const GLfloat h = 0.5f;

    glBegin(GL_QUADS);

    // Front (+Z)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-h, -h,  h);
    glVertex3f( h, -h,  h);
    glVertex3f( h,  h,  h);
    glVertex3f(-h,  h,  h);

    // Back (-Z)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f( h, -h, -h);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h,  h, -h);
    glVertex3f( h,  h, -h);

    // Left (-X)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, -h,  h);
    glVertex3f(-h,  h,  h);
    glVertex3f(-h,  h, -h);

    // Right (+X)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( h, -h,  h);
    glVertex3f( h, -h, -h);
    glVertex3f( h,  h, -h);
    glVertex3f( h,  h,  h);

    // Top (+Y)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-h,  h,  h);
    glVertex3f( h,  h,  h);
    glVertex3f( h,  h, -h);
    glVertex3f(-h,  h, -h);

    // Bottom (-Y)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f( h, -h, -h);
    glVertex3f( h, -h,  h);
    glVertex3f(-h, -h,  h);

    glEnd();
}

// Applies the cube material.
// All cubes share ambient/diffuse/specular; only shininess changes.
static void ApplyCubeMaterial(float shininess) {
    // Blue diffuse base color.
    const GLfloat diffuse[4]  = { 0.18f, 0.38f, 0.90f, 1.0f };

    // Ambient is approximately 20% of diffuse, as requested.
    const GLfloat ambient[4]  = {
        diffuse[0] * 0.20f,
        diffuse[1] * 0.20f,
        diffuse[2] * 0.20f,
        1.0f
    };

    // White specular for visible highlight on every cube.
    const GLfloat specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

    // In fixed-function OpenGL, shininess is defined in [0, 128].
    // Lower values -> broad, soft highlight; higher values -> tighter, sharper hotspot.
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

// Draws all cube labels in screen space so they stay crisp and avoid depth conflicts.
static void DrawLabelsOverlay(const GLdouble model[16],
                              const GLdouble proj[16],
                              const GLint viewport[4]) {
    const void* font = GLUT_BITMAP_HELVETICA_18;

    // Disable lighting/depth for flat overlay text.
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Switch to pixel-like 2D projection.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(gWindowWidth),
               0.0, static_cast<double>(gWindowHeight));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Light gray/white labels.
    glColor3f(0.93f, 0.93f, 0.93f);

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const float x = CubeCenterX(col);
            const float y = CubeCenterY(row) - (kCubeSize * 0.80f + 0.45f);
            const float z = 0.0f;

            GLdouble sx = 0.0, sy = 0.0, sz = 0.0;
            gluProject(static_cast<GLdouble>(x),
                       static_cast<GLdouble>(y),
                       static_cast<GLdouble>(z),
                       model, proj, viewport,
                       &sx, &sy, &sz);

            const char* label = kLabelText[row][col];
            const int textWidth = BitmapStringWidth(const_cast<void*>(font), label);

            // Center text under each cube.
            const float drawX = static_cast<float>(sx) - (textWidth * 0.5f);
            const float drawY = static_cast<float>(sy) - 8.0f;

            DrawBitmapString2D(drawX, drawY, const_cast<void*>(font), label);
        }
    }

    // Restore matrices and states.
    glPopMatrix(); // modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Initializes OpenGL fixed-function render state.
static void init() {
    // Flat dark gray background to match reference.
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    // Requested fixed-function states.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // Keeps normals unit-length after scaling.
    glShadeModel(GL_SMOOTH);

    // Keep scene ambient low so directional shading is visible.
    const GLfloat globalAmbient[4] = { 0.00f, 0.00f, 0.00f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Light0 properties (explicit ambient/diffuse/specular).
    const GLfloat lightAmbient[4]  = { 0.12f, 0.12f, 0.12f, 1.0f };
    const GLfloat lightDiffuse[4]  = { 0.95f, 0.95f, 0.95f, 1.0f };
    const GLfloat lightSpecular[4] = { 1.00f, 1.00f, 1.00f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Specular response is a bit more intuitive with local viewer enabled.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

// Reshape callback: updates viewport and perspective projection.
static void reshape(int w, int h) {
    gWindowWidth  = (w > 0) ? w : 1;
    gWindowHeight = (h > 0) ? h : 1;

    glViewport(0, 0, gWindowWidth, gWindowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Slight perspective view similar to the reference image.
    const double aspect = static_cast<double>(gWindowWidth) / static_cast<double>(gWindowHeight);
    gluPerspective(45.0, aspect, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

// Main display callback.
static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera setup: mild perspective and slight vertical offset.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        0.60, 0.35, 13.5,  // eye
        0.00, 0.00,  0.0,  // center
        0.00, 1.00,  0.0   // up
    );

    // Directional light (w = 0): from above-left-front.
    // This keeps front faces lit and right faces comparatively darker.
    const GLfloat lightDir[4] = { -0.70f, 0.85f, 1.00f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightDir);

    // Draw all 8 cubes.
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const int requestedShininess = kShininessLabels[row][col];

            // Legacy OpenGL shininess range is [0,128].
            // Keep label "256" visible, but clamp GL value to 128 for rendering.
            const float glShininess = static_cast<float>(std::min(requestedShininess, 128));

            ApplyCubeMaterial(glShininess);

            glPushMatrix();

            // Place cube in the 2x4 world-space grid.
            glTranslatef(CubeCenterX(col), CubeCenterY(row), 0.0f);

            // Rotate so front and right faces are both visible, matching the reference style.
            glRotatef(-24.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(  7.0f, 1.0f, 0.0f, 0.0f);

            // Uniform scaling from unit cube to desired size.
            glScalef(kCubeSize, kCubeSize, kCubeSize);
            DrawUnitCube();

            glPopMatrix();
        }
    }

    // Capture current 3D transforms to project label anchor points.
    GLdouble model[16];
    GLdouble proj[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Draw numeric shininess labels under each cube.
    DrawLabelsOverlay(model, proj, viewport);

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(gWindowWidth, gWindowHeight);
    glutCreateWindow("Specular Lighting, Objects, Illumination and Shaders");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}

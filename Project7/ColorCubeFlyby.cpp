// This program is a flyby around the RGB color cube.  One intersting note
// is that because the cube is a convex polyhedron and it is the only thing
// in the scene, we can render it using backface culling only. i.e., there
// is no need for a depth buffer.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <cstdlib>

// Global control variables
GLfloat rotX = 0.0, rotY = 0.0, rotZ = 0.0;
bool cameraPaused = false;
GLfloat panY = 0.0;              // Vertical pan (up/down)
GLfloat zoomDistance = 1.0;      // Camera distance multiplier (1.0 = normal, <1.0 = closer, >1.0 = farther)
int windowWidth = 500, windowHeight = 500;  // Window dimensions

// Bouncing cubes state
struct BouncingCube {
  GLfloat x, y, z;          // Position
  GLfloat vx, vy, vz;       // Velocity
  GLfloat r, g, b;          // Color
  GLfloat brightness;       // Brightness multiplier
  GLfloat size;             // Size of cube
};

// Prism boundaries (will be calculated in init)
GLfloat PRISM_LEFT_EDGE = 0.0f;
GLfloat PRISM_RIGHT_EDGE = 0.0f;
GLfloat PRISM_DISTANCE = 0.0f;
GLfloat CUBE_SPEED = 0.06f;

BouncingCube cube1;  // Red (above)
BouncingCube cube2;  // Green (below)

// The cube has opposite corners at (0,0,0) and (1,1,1), which are black and
// white respectively.  The x-axis is the red gradient, the y-axis is the
// green gradient, and the z-axis is the blue gradient.  The cube's position
// and colors are fixed.
namespace Cube {

const int NUM_VERTICES = 8;
const int NUM_FACES = 6;

GLint vertices[NUM_VERTICES][3] = {
  {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
  {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}};

GLint faces[NUM_FACES][4] = {
  {1, 5, 7, 3}, {5, 4, 6, 7}, {4, 0, 2, 6},
  {3, 7, 6, 2}, {0, 1, 3, 2}, {0, 4, 5, 1}};

GLfloat vertexColors[NUM_VERTICES][3] = {
  {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
  {1.0, 0.0, 0.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}};

void draw() {
  // Apply rotation around cube center
  glTranslatef(0.5, 0.5, 0.5);
  glRotatef(rotX, 1.0, 0.0, 0.0);
  glRotatef(rotY, 0.0, 1.0, 0.0);
  glRotatef(rotZ, 0.0, 0.0, 1.0);
  glTranslatef(-0.5, -0.5, -0.5);

  glBegin(GL_QUADS);
  for (int i = 0; i < NUM_FACES; i++) {
    for (int j = 0; j < 4; j++) {
      glColor3fv((GLfloat*)&vertexColors[faces[i][j]]);
      glVertex3iv((GLint*)&vertices[faces[i][j]]);
    }
  }
  glEnd();
}
}

// Two rectangular prisms positioned to the left and right of the cube
namespace Planes {

// Solid blue color: RGB(66, 135, 245) normalized to [0, 1]
const GLfloat SOLID_COLOR[3] = {66.0/255.0, 135.0/255.0, 245.0/255.0};

// PRISM PARAMETERS - Modify these to change position and scale
// Left prism: position (x, y, z) and dimensions (width, height, depth)
const GLfloat LEFT_POS_X = -6.5;
const GLfloat LEFT_POS_Y = -3.5;
const GLfloat LEFT_POS_Z = 0.0;
const GLfloat LEFT_WIDTH = 0.1;   // X dimension
const GLfloat LEFT_HEIGHT = 7.0;  // Y dimension
const GLfloat LEFT_DEPTH = 2.0;   // Z dimension

// Right prism: position (x, y, z) and dimensions (width, height, depth)
const GLfloat RIGHT_POS_X = 7.5;
const GLfloat RIGHT_POS_Y = -3.5;
const GLfloat RIGHT_POS_Z = 0.0;
const GLfloat RIGHT_WIDTH = 0.1;   // X dimension
const GLfloat RIGHT_HEIGHT = 7.0;  // Y dimension
const GLfloat RIGHT_DEPTH = 2.0;   // Z dimension

const int NUM_VERTICES = 16;  // 8 vertices per prism
const int NUM_FACES = 12;     // 6 faces per prism

GLfloat vertices[NUM_VERTICES][3];
GLint faces[NUM_FACES][4] = {
  // Left prism faces
  {0, 1, 2, 3}, {4, 7, 6, 5}, {0, 4, 5, 1}, {3, 2, 6, 7}, {0, 3, 7, 4}, {1, 5, 6, 2},
  // Right prism faces
  {8, 9, 10, 11}, {12, 13, 14, 15}, {8, 12, 13, 9}, {11, 10, 14, 15}, {8, 11, 15, 12}, {9, 13, 14, 10}
};

void initGeometry() {
  // Left prism vertices
  vertices[0][0] = LEFT_POS_X;           vertices[0][1] = LEFT_POS_Y;            vertices[0][2] = LEFT_POS_Z;
  vertices[1][0] = LEFT_POS_X + LEFT_WIDTH; vertices[1][1] = LEFT_POS_Y;            vertices[1][2] = LEFT_POS_Z;
  vertices[2][0] = LEFT_POS_X + LEFT_WIDTH; vertices[2][1] = LEFT_POS_Y + LEFT_HEIGHT; vertices[2][2] = LEFT_POS_Z;
  vertices[3][0] = LEFT_POS_X;           vertices[3][1] = LEFT_POS_Y + LEFT_HEIGHT; vertices[3][2] = LEFT_POS_Z;
  vertices[4][0] = LEFT_POS_X;           vertices[4][1] = LEFT_POS_Y;            vertices[4][2] = LEFT_POS_Z + LEFT_DEPTH;
  vertices[5][0] = LEFT_POS_X + LEFT_WIDTH; vertices[5][1] = LEFT_POS_Y;            vertices[5][2] = LEFT_POS_Z + LEFT_DEPTH;
  vertices[6][0] = LEFT_POS_X + LEFT_WIDTH; vertices[6][1] = LEFT_POS_Y + LEFT_HEIGHT; vertices[6][2] = LEFT_POS_Z + LEFT_DEPTH;
  vertices[7][0] = LEFT_POS_X;           vertices[7][1] = LEFT_POS_Y + LEFT_HEIGHT; vertices[7][2] = LEFT_POS_Z + LEFT_DEPTH;

  // Right prism vertices
  vertices[8][0] = RIGHT_POS_X;            vertices[8][1] = RIGHT_POS_Y;             vertices[8][2] = RIGHT_POS_Z;
  vertices[9][0] = RIGHT_POS_X + RIGHT_WIDTH;  vertices[9][1] = RIGHT_POS_Y;             vertices[9][2] = RIGHT_POS_Z;
  vertices[10][0] = RIGHT_POS_X + RIGHT_WIDTH; vertices[10][1] = RIGHT_POS_Y + RIGHT_HEIGHT; vertices[10][2] = RIGHT_POS_Z;
  vertices[11][0] = RIGHT_POS_X;           vertices[11][1] = RIGHT_POS_Y + RIGHT_HEIGHT; vertices[11][2] = RIGHT_POS_Z;
  vertices[12][0] = RIGHT_POS_X;           vertices[12][1] = RIGHT_POS_Y;             vertices[12][2] = RIGHT_POS_Z + RIGHT_DEPTH;
  vertices[13][0] = RIGHT_POS_X + RIGHT_WIDTH; vertices[13][1] = RIGHT_POS_Y;             vertices[13][2] = RIGHT_POS_Z + RIGHT_DEPTH;
  vertices[14][0] = RIGHT_POS_X + RIGHT_WIDTH; vertices[14][1] = RIGHT_POS_Y + RIGHT_HEIGHT; vertices[14][2] = RIGHT_POS_Z + RIGHT_DEPTH;
  vertices[15][0] = RIGHT_POS_X;           vertices[15][1] = RIGHT_POS_Y + RIGHT_HEIGHT; vertices[15][2] = RIGHT_POS_Z + RIGHT_DEPTH;
}

void draw() {
  glBegin(GL_QUADS);
  for (int i = 0; i < NUM_FACES; i++) {
    for (int j = 0; j < 4; j++) {
      glColor3fv(SOLID_COLOR);
      glVertex3fv((GLfloat*)&vertices[faces[i][j]]);
    }
  }
  glEnd();
}
}

// Bouncing cubes namespace
namespace BouncingCubes {

void drawCube(const BouncingCube& cube) {
  glPushMatrix();
  glTranslatef(cube.x, cube.y, cube.z);

  GLfloat halfSize = cube.size / 2.0f;

  // Vertex colors like the center cube: each axis maps to a color channel
  // Based on whether vertex is at -halfSize (0) or +halfSize (1)
  GLfloat colors[8][3] = {
    {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
    {1.0, 0.0, 0.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}
  };

  // Apply brightness to all vertex colors
  for (int i = 0; i < 8; i++) {
    colors[i][0] *= cube.brightness;
    colors[i][1] *= cube.brightness;
    colors[i][2] *= cube.brightness;
  }

  glBegin(GL_QUADS);

  // Front face (Z+): vertices 0,1,3,2
  glColor3fv(colors[0]); glVertex3f(-halfSize, -halfSize, halfSize);
  glColor3fv(colors[1]); glVertex3f(halfSize, -halfSize, halfSize);
  glColor3fv(colors[3]); glVertex3f(halfSize, halfSize, halfSize);
  glColor3fv(colors[2]); glVertex3f(-halfSize, halfSize, halfSize);

  // Back face (Z-): vertices 4,7,5,6 (reversed winding)
  glColor3fv(colors[4]); glVertex3f(-halfSize, -halfSize, -halfSize);
  glColor3fv(colors[7]); glVertex3f(-halfSize, halfSize, -halfSize);
  glColor3fv(colors[5]); glVertex3f(halfSize, halfSize, -halfSize);
  glColor3fv(colors[6]); glVertex3f(halfSize, -halfSize, -halfSize);

  // Top face (Y+): vertices 2,3,7,6
  glColor3fv(colors[2]); glVertex3f(-halfSize, halfSize, -halfSize);
  glColor3fv(colors[3]); glVertex3f(-halfSize, halfSize, halfSize);
  glColor3fv(colors[7]); glVertex3f(halfSize, halfSize, halfSize);
  glColor3fv(colors[6]); glVertex3f(halfSize, halfSize, -halfSize);

  // Bottom face (Y-): vertices 0,4,5,1
  glColor3fv(colors[0]); glVertex3f(-halfSize, -halfSize, -halfSize);
  glColor3fv(colors[4]); glVertex3f(halfSize, -halfSize, -halfSize);
  glColor3fv(colors[5]); glVertex3f(halfSize, -halfSize, halfSize);
  glColor3fv(colors[1]); glVertex3f(-halfSize, -halfSize, halfSize);

  // Right face (X+): vertices 4,6,7,5
  glColor3fv(colors[4]); glVertex3f(halfSize, -halfSize, -halfSize);
  glColor3fv(colors[6]); glVertex3f(halfSize, halfSize, -halfSize);
  glColor3fv(colors[7]); glVertex3f(halfSize, halfSize, halfSize);
  glColor3fv(colors[5]); glVertex3f(halfSize, -halfSize, halfSize);

  // Left face (X-): vertices 0,1,3,2
  glColor3fv(colors[0]); glVertex3f(-halfSize, -halfSize, -halfSize);
  glColor3fv(colors[1]); glVertex3f(-halfSize, -halfSize, halfSize);
  glColor3fv(colors[3]); glVertex3f(-halfSize, halfSize, halfSize);
  glColor3fv(colors[2]); glVertex3f(-halfSize, halfSize, -halfSize);

  glEnd();
  glPopMatrix();
}

void draw() {
  drawCube(cube1);
  drawCube(cube2);
}

void update() {
  // Update positions
  cube1.x += cube1.vx;
  cube2.x += cube2.vx;

  // Cube1 bouncing at boundaries
  if (cube1.x + cube1.size/2.0f > PRISM_RIGHT_EDGE) {
    cube1.x = PRISM_RIGHT_EDGE - cube1.size/2.0f;
    cube1.vx = -cube1.vx;
  }
  if (cube1.x - cube1.size/2.0f < PRISM_LEFT_EDGE) {
    cube1.x = PRISM_LEFT_EDGE + cube1.size/2.0f;
    cube1.vx = -cube1.vx;
  }

  // Cube2 bouncing at boundaries
  if (cube2.x + cube2.size/2.0f > PRISM_RIGHT_EDGE) {
    cube2.x = PRISM_RIGHT_EDGE - cube2.size/2.0f;
    cube2.vx = -cube2.vx;
  }
  if (cube2.x - cube2.size/2.0f < PRISM_LEFT_EDGE) {
    cube2.x = PRISM_LEFT_EDGE + cube2.size/2.0f;
    cube2.vx = -cube2.vx;
  }
}

void initBouncingCubes() {
  // Calculate prism boundaries for bouncing
  // LEFT prism: from -6.5 to -6.4, RIGHT prism: from 7.5 to 7.6
  PRISM_LEFT_EDGE = -6.5f;           // Left boundary to bounce at
  PRISM_RIGHT_EDGE = 7.5f;           // Right boundary to bounce at
  PRISM_DISTANCE = PRISM_RIGHT_EDGE - PRISM_LEFT_EDGE;

  // Top cube: above, starts at left, moves right
  // Gray (center cube color), less illumination/brightness
  cube1.x = PRISM_LEFT_EDGE + 0.5f;
  cube1.y = 1.5f;
  cube1.z = 0.5f;
  cube1.vx = CUBE_SPEED;
  cube1.vy = 0.0f;
  cube1.vz = 0.0f;
  cube1.r = 0.5f; cube1.g = 0.5f; cube1.b = 0.5f;
  cube1.brightness = 0.5f;
  cube1.size = 0.25f;

  // Bottom cube: below, starts at right, moves left
  // Gray (center cube color), more illumination/brightness
  cube2.x = PRISM_RIGHT_EDGE - 0.5f;
  cube2.y = -1.5f;
  cube2.z = 0.5f;
  cube2.vx = -CUBE_SPEED;
  cube2.vy = 0.0f;
  cube2.vz = 0.0f;
  cube2.r = 0.5f; cube2.g = 0.5f; cube2.b = 0.5f;
  cube2.brightness = 1.5f;
  cube2.size = 0.25f;
}
}

// Display and Animation. To draw we just clear the window and draw the cube.
// Because our main window is double buffered we have to swap the buffers to
// make the drawing visible. Animation is achieved by successively moving our
// camera and drawing. The function nextAnimationFrame() moves the camera to
// the next point and draws. The way that we get animation in OpenGL is to
// register nextFrame as the idle function; this is done in main().
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Cube::draw();
  Planes::draw();
  BouncingCubes::draw();
  glFlush();
  glutSwapBuffers();
}

// We'll be flying around the cube by moving the camera along the orbit of the
// curve u->(8*cos(u), 7*cos(u)-1, 4*cos(u/3)+2).  We keep the camera looking
// at the center of the cube (0.5, 0.5, 0.5) and vary the up vector to achieve
// a weird tumbling effect.
void timer(int v) {
  static GLfloat u = 0.0;

  // Update camera position if not paused
  if (!cameraPaused) {
    u += 0.01;
    // Update bouncing cubes only when camera is moving
    BouncingCubes::update();
  }

  glLoadIdentity();
  // Scale camera distance from cube center by zoomDistance
  GLfloat camX = 8*cos(u) * zoomDistance;
  GLfloat camY = (7*cos(u) - 1) * zoomDistance + panY;
  GLfloat camZ = (4*cos(u/3) + 2) * zoomDistance;
  gluLookAt(camX, camY, camZ, .5, .5+panY, .5, cos(u), 1, 0);
  glutPostRedisplay();
  glutTimerFunc(1000/60.0, timer, v);
}

// Update projection matrix
void updateProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, GLfloat(windowWidth) / GLfloat(windowHeight), 0.5, 40.0);
  glMatrixMode(GL_MODELVIEW);
}

// Keyboard handler for user controls
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'r':
    case 'R':
      // Rotate cube 15 degrees left-right (X axis)
      rotZ += 15.0;
      break;
    case 's':
    case 'S':
      // Stop camera movement
      cameraPaused = true;
      break;
    case 'c':
    case 'C':
      // Continue camera movement
      cameraPaused = false;
      break;
    case 'u':
    case 'U':
      // Move image up
      panY += 0.5;
      break;
    case 'd':
    case 'D':
      // Move image down
      panY -= 0.5;
      break;
    case '+':
    case '=':
      // Zoom in (move camera closer)
      zoomDistance /= 1.2;
      if (zoomDistance < 0.2) zoomDistance = 0.2;  // Minimum zoom
      break;
    case '-':
    case '_':
      // Zoom out (move camera farther)
      zoomDistance *= 1.2;
      if (zoomDistance > 5.0) zoomDistance = 5.0;  // Maximum zoom
      break;
    case 27:  // ESC key to exit
      exit(0);
      break;
  }
}

// When the window is reshaped we have to recompute the camera settings to
// match the new window shape.  Set the viewport to (0,0)-(w,h).  Set the
// camera to have a 60 degree vertical field of view, aspect ratio w/h, near
// clipping plane distance 0.5 and far clipping plane distance 40.
void reshape(int w, int h) {
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, w, h);
  updateProjection();
}

// Application specific initialization:  The only thing we really need to do
// is enable back face culling because the only thing in the scene is a cube
// which is a convex polyhedron.
void init() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  Planes::initGeometry();
  BouncingCubes::initBouncingCubes();
}

// The usual main for a GLUT application.
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutCreateWindow("The RGB Color Cube - Controls: R=rotate, S=stop camera, C=continue camera, +/-=zoom, U/D=pan, ESC=exit");

  glutReshapeFunc(reshape);
  glutTimerFunc(100, timer, 0);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
}

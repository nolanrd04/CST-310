// This application is a trivial illustration of texture mapping.  It draws
// several triangles, each with a texture mapped on to it.  The same texture
// is used for each triangle, but the mappings vary quite a bit so it looks as
// if each triangle has a different texture.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cstdlib>

// Define a 2 x 2 red and yellow checkered pattern using RGB colors.
#define red {0xff, 0x00, 0x00}
#define yellow {0xff, 0xff, 0x00}
GLubyte texture[][3] = {
    red, yellow,
    yellow, red,
};

GLfloat rotationAngle = 0.0f;
GLfloat xOffset = 0.0f;
GLfloat yOffset = 0.0f;
GLfloat zoom = 1.0f;
bool spinning = true;

const GLfloat ROTATION_STEP = 1.0f;
const GLfloat MOVE_STEP = 0.2f;
const GLfloat ZOOM_STEP = 0.1f;
const GLfloat MIN_ZOOM = 0.2f;
const GLfloat MAX_ZOOM = 4.0f;

void initTexture() {
  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D,
               0,                    // level 0
               3,                    // use only R, G, and B components
               2, 2,                 // texture has 2x2 texels
               0,                    // no border
               GL_RGB,               // texels are in RGB format
               GL_UNSIGNED_BYTE,     // color components are unsigned bytes
               texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

// Fixes projection when window reshaped.
void reshape(int width, int height) {
  if (height == 0) {
    height = 1;
  }

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  GLfloat aspect = GLfloat(width) / GLfloat(height);
  if (aspect >= 1.0f) {
    glOrtho(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f, -1.0f, 1.0f);
  } else {
    glOrtho(-4.0f, 4.0f, -4.0f / aspect, 4.0f / aspect, -1.0f, 1.0f);
  }

  glMatrixMode(GL_MODELVIEW);
}

// Draws three textured triangles.  Each triangle uses the same texture,
// but the mappings of texture coordinates to vertex coordinates is
// different in each triangle.
void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Rotation is around the origin so the object spins like a windmill.
  glTranslatef(xOffset, yOffset, 0.0f);
  glScalef(zoom, zoom, 1.0f);
  glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);

  glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5, 1.0);    glVertex2f(-3, 3);
    glTexCoord2f(0.0, 0.0);    glVertex2f(-3, 0);
    glTexCoord2f(1.0, 0.0);    glVertex2f(0, 0);

    glTexCoord2f(4, 8);        glVertex2f(3, 3);
    glTexCoord2f(0.0, 0.0);    glVertex2f(0, 0);
    glTexCoord2f(8, 0.0);      glVertex2f(3, 0);

    glTexCoord2f(5, 5);        glVertex2f(0, 0);
    glTexCoord2f(0.0, 0.0);    glVertex2f(-1.5, -3);
    glTexCoord2f(4, 0.0);      glVertex2f(1.5, -3);
  glEnd();
  glutSwapBuffers();
}

void timer(int value) {
  if (spinning) {
    rotationAngle += ROTATION_STEP;
    if (rotationAngle >= 360.0f) {
      rotationAngle -= 360.0f;
    }
  }

  glutPostRedisplay();
  glutTimerFunc(1000 / 60, timer, value);
}

void keyboard(unsigned char key, int x, int y) {
  (void)x;
  (void)y;

  switch (key) {
    case 'p':
    case 'P':
      spinning = false;
      break;
    case 'c':
    case 'C':
      spinning = true;
      break;
    case 'd':
    case 'D':
      yOffset += MOVE_STEP;
      break;
    case 'u':
    case 'U':
      yOffset -= MOVE_STEP;
      break;
    case 'r':
    case 'R':
      xOffset -= MOVE_STEP;
      break;
    case 'l':
    case 'L':
      xOffset += MOVE_STEP;
      break;
    case '+':
    case '=':
      zoom += ZOOM_STEP;
      if (zoom > MAX_ZOOM) {
        zoom = MAX_ZOOM;
      }
      break;
    case '-':
    case '_':
      zoom -= ZOOM_STEP;
      if (zoom < MIN_ZOOM) {
        zoom = MIN_ZOOM;
      }
      break;
    case 27:  // Escape key.
      std::exit(0);
    default:
      return;
  }

  glutPostRedisplay();
}


#include <cctype>
#include <functional>
#include <unordered_map>

struct ControlBinding {
  const char* actionName;
  const char* description;
  std::function<void()> run;
};

static const std::unordered_map<char, ControlBinding> kControls = {
  {'p', {"pause_spin", "pause spinning", [] { spinning = false; }}},
  {'c', {"resume_spin", "continue spinning", [] { spinning = true; }}},
  {'d', {"move_up", "move up", [] { yOffset += MOVE_STEP; }}},
  {'u', {"move_down", "move down", [] { yOffset -= MOVE_STEP; }}},
  {'r', {"move_right", "move right", [] { xOffset -= MOVE_STEP; }}},
  {'l', {"move_left", "move left", [] { xOffset += MOVE_STEP; }}},
  {'+', {"zoom_in", "zoom in", [] {
    zoom += ZOOM_STEP;
    if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
  }}},
  {'-', {"zoom_out", "zoom out", [] {
    zoom -= ZOOM_STEP;
    if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
  }}},
  {'\x1b', {"exit", "exit app", [] { std::exit(0); }}},
};

void keyboard(unsigned char key, int, int) {
  char normalized = static_cast<char>(std::tolower(static_cast<unsigned char>(key)));
  if (key == '=') normalized = '+';  // alias
  if (key == '_') normalized = '-';  // alias

  auto it = kControls.find(normalized);
  if (it != kControls.end()) it->second.run();
  glutPostRedisplay();
}

// Initializes GLUT and enters the main loop.
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(520, 390);
  glutCreateWindow("Textured Triangles");
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  initTexture();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(0, timer, 0);
  glutMainLoop();
}

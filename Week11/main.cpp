#include <GL/glut.h>

// Draw one flat plane centered at origin.
void drawPlane() {
  glColor3f(0.29f, 0.64f, 1.0f);
  glBegin(GL_QUADS);
  glNormal3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-2.0f, 0.0f, -2.0f);
  glVertex3f(2.0f, 0.0f, -2.0f);
  glVertex3f(2.0f, 0.0f, 2.0f);
  glVertex3f(-2.0f, 0.0f, 2.0f);
  glEnd();
}

// Render one frame.
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Camera position and target.
  gluLookAt(
      0.0, 2.5, 4.0,
      0.0, 0.0, 0.0,
      0.0, 1.0, 0.0);

  // Basic key light.
  GLfloat lightPos[] = {2.0f, 4.0f, 2.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  drawPlane();

  glutSwapBuffers();
}

// Keep projection correct when the window changes.
void reshape(int width, int height) {
  if (height == 0) {
    height = 1;
  }

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, static_cast<double>(width) / static_cast<double>(height), 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
}

// Minimal OpenGL state.
void initGL() {
  glClearColor(0.12f, 0.14f, 0.19f, 1.0f);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  GLfloat ambient[] = {0.25f, 0.25f, 0.25f, 1.0f};
  GLfloat diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(1280, 720);
  glutCreateWindow("Basic OpenGL Plane");

  initGL();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc([]() { glutPostRedisplay(); });

  glutMainLoop();
  return 0;
}

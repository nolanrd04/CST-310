#include <GL/glut.h>

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.0f);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Light position (directional light)
    GLfloat lightPos[] = {0.0f, 2.0f, 1.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Material color
    GLfloat diffuseColor[] = {0.8f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

    // Draw object
    glutSolidSphere(1.0, 50, 50);

    glutSwapBuffers();
}

void reshape(int width, int height) {
    if (height == 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, static_cast<double>(width) / height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Lighting Sphere");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();

    return 0;
}

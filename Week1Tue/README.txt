cd Week1Tue

1. Navigate to the root directory (where you extract the zip file to)
example: cd ..Project1

2. Install OpenGL if not already (use sudo if not in root)
sudo apt update && sudo apt install -y build-essential libglu1-mesa-dev freeglut3-dev mesa-common-dev libgl1-mesa-dev

3. Compile the program:
g++ TestOpenGL.cpp -o myFirstOpenGL -lglut -lGLU -lGL

3.5. (WINDOWS ONLY) Set the target display:
export DISPLAY=:0

4. Run the program:
./myFirstOpenGL


PACKAGES USED:
1. GLUT: #include <GL/glut.h>
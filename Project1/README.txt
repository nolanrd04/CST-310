1. Navigate to the root directory (where you extract the zip file to)
example: cd ..Project1

2. Install OpenGL if not already (use sudo if not in root)
sudo apt update && sudo apt install -y build-essential libglu1-mesa-dev freeglut3-dev mesa-common-dev libgl1-mesa-dev

3. Compile the 2d program:
g++ Sierpinski.cpp -o sp2d -lglut -lGLU -lGL

4. Start X server

5. Set the display: 
export DISPLAY=:0

or make it persistent:

echo "export DISPLAY=:0" >> ~/.bashrc

6. Run the 2d program:
./sp2d

7. Remove compiled program
rm sp2d

8. Compile the 3d program:
g++ Sierpinski3d.cpp -o sp3d -lglut -lGLU -lGL

9. Run the 3d program:
./sp3d

10. Remove the compiled program
rm sp3d

-------------------


PACKAGES USED:
1. GLUT: #include <GL/glut.h>
2. CstdLib: #include <cstdlib>
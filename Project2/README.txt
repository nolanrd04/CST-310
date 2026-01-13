1. Navigate to the root directory (where you extract the zip file to)
example: cd ..Project1

2. Install OpenGL if not already (use sudo if not in root)
sudo apt update && sudo apt install -y build-essential libglu1-mesa-dev freeglut3-dev mesa-common-dev libgl1-mesa-dev

---- OPTIONAL STEPS IF X SERVER ISN'T SET UP ----
3. Start X server

4. Set the display: 
export DISPLAY=:0

or make it persistent:

echo "export DISPLAY=:0" >> ~/.bashrc
-------------------------------------------------

5. Compile the first program:
g++ SceneOne.cpp -o s1 -lglut -lGLU -lGL

6. Run the 2d program:
./s1

7. Remove compiled program
rm s1

8. Compile the second program:
g++ SceneTwo.cpp -o s2 -lglut -lGLU -lGL

9. Run the second program:
./s2

10. Remove the compiled program
rm s2

-------------------


PACKAGES USED:
1. GLUT: #include <GL/glut.h>
2. CstdLib: #include <cstdlib>
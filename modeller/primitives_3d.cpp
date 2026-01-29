#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

/* ============================================================================
   EASY 3D PRIMITIVE SHAPE FRAMEWORK
   
   This framework allows you to easily:
   1. Create primitive shapes (cube, sphere, plane, cylinder, etc.)
   2. Place them anywhere in 3D space
   3. Set their color, position, rotation, and scale
   4. Render multiple objects in one scene
   ============================================================================ */

// Vertex Shader - same as before
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// Fragment Shader - same as before
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

/* ============================================================================
   MESH STRUCTURE
   
   Stores vertex data and OpenGL buffer objects for a shape
   ============================================================================ */
struct Mesh {
    std::vector<float> vertices;      // Position + Normal data
    std::vector<unsigned int> indices; // Triangle indices
    GLuint VAO, VBO, EBO;
    
    // Constructor initializes OpenGL buffers
    Mesh() : VAO(0), VBO(0), EBO(0) {}
    
    // Upload mesh data to GPU
    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    // Draw the mesh
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    // Cleanup GPU resources
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

/* ============================================================================
   OBJECT STRUCTURE
   
   Represents a 3D object with transform properties and color
   ============================================================================ */
struct Object3D {
    Mesh* mesh;                          // Pointer to shared mesh data
    glm::vec3 position;                  // Position in world space
    glm::vec3 rotation;                  // Rotation in degrees (x, y, z)
    glm::vec3 scale;                     // Scale (1.0 = normal size)
    glm::vec3 color;                     // RGB color (0.0 to 1.0)
    
    // Constructor with default values
    Object3D(Mesh* m, glm::vec3 pos = glm::vec3(0.0f), 
             glm::vec3 col = glm::vec3(0.7f, 0.7f, 0.7f))
        : mesh(m), position(pos), rotation(0.0f), scale(1.0f), color(col) {}
    
    // Calculate the model matrix for this object
    glm::mat4 getModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        
        // Apply transformations in order: Scale → Rotate → Translate
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        
        return model;
    }
};

/* ============================================================================
   PRIMITIVE SHAPE GENERATORS
   
   Functions to create common 3D shapes
   ============================================================================ */

// PLANE (flat rectangle in XZ plane)
Mesh createPlane(float width = 2.0f, float depth = 2.0f, int subdivisionsW = 10, int subdivisionsD = 10) {
    Mesh mesh;
    
    for (int z = 0; z <= subdivisionsD; z++) {
        for (int x = 0; x <= subdivisionsW; x++) {
            float xPos = (float)x / subdivisionsW * width - width / 2.0f;
            float zPos = (float)z / subdivisionsD * depth - depth / 2.0f;
            
            // Position
            mesh.vertices.push_back(xPos);
            mesh.vertices.push_back(0.0f);  // Y = 0 (flat plane)
            mesh.vertices.push_back(zPos);
            
            // Normal (pointing up)
            mesh.vertices.push_back(0.0f);
            mesh.vertices.push_back(1.0f);
            mesh.vertices.push_back(0.0f);
        }
    }
    
    // Generate indices (same logic as curtain)
    for (int z = 0; z < subdivisionsD; z++) {
        for (int x = 0; x < subdivisionsW; x++) {
            int topLeft = z * (subdivisionsW + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (subdivisionsW + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            mesh.indices.push_back(topLeft);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(topRight);
            
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(bottomRight);
        }
    }
    
    mesh.setupBuffers();
    return mesh;
}

// CUBE (box with 6 faces)
Mesh createCube(float size = 1.0f) {
    Mesh mesh;
    float s = size / 2.0f;  // Half size for centering
    
    // Each face needs its own vertices because normals are different
    // Format: position (x,y,z), normal (nx,ny,nz)
    float cubeVertices[] = {
        // Front face (normal = +Z)
        -s, -s,  s,  0.0f, 0.0f, 1.0f,
         s, -s,  s,  0.0f, 0.0f, 1.0f,
         s,  s,  s,  0.0f, 0.0f, 1.0f,
        -s,  s,  s,  0.0f, 0.0f, 1.0f,
        
        // Back face (normal = -Z)
         s, -s, -s,  0.0f, 0.0f, -1.0f,
        -s, -s, -s,  0.0f, 0.0f, -1.0f,
        -s,  s, -s,  0.0f, 0.0f, -1.0f,
         s,  s, -s,  0.0f, 0.0f, -1.0f,
        
        // Left face (normal = -X)
        -s, -s, -s,  -1.0f, 0.0f, 0.0f,
        -s, -s,  s,  -1.0f, 0.0f, 0.0f,
        -s,  s,  s,  -1.0f, 0.0f, 0.0f,
        -s,  s, -s,  -1.0f, 0.0f, 0.0f,
        
        // Right face (normal = +X)
         s, -s,  s,  1.0f, 0.0f, 0.0f,
         s, -s, -s,  1.0f, 0.0f, 0.0f,
         s,  s, -s,  1.0f, 0.0f, 0.0f,
         s,  s,  s,  1.0f, 0.0f, 0.0f,
        
        // Top face (normal = +Y)
        -s,  s,  s,  0.0f, 1.0f, 0.0f,
         s,  s,  s,  0.0f, 1.0f, 0.0f,
         s,  s, -s,  0.0f, 1.0f, 0.0f,
        -s,  s, -s,  0.0f, 1.0f, 0.0f,
        
        // Bottom face (normal = -Y)
        -s, -s, -s,  0.0f, -1.0f, 0.0f,
         s, -s, -s,  0.0f, -1.0f, 0.0f,
         s, -s,  s,  0.0f, -1.0f, 0.0f,
        -s, -s,  s,  0.0f, -1.0f, 0.0f
    };
    
    // Copy vertices to mesh
    for (int i = 0; i < 144; i++) {
        mesh.vertices.push_back(cubeVertices[i]);
    }
    
    // Indices for 6 faces (2 triangles per face)
    for (int i = 0; i < 6; i++) {
        int offset = i * 4;
        mesh.indices.push_back(offset + 0);
        mesh.indices.push_back(offset + 1);
        mesh.indices.push_back(offset + 2);
        mesh.indices.push_back(offset + 0);
        mesh.indices.push_back(offset + 2);
        mesh.indices.push_back(offset + 3);
    }
    
    mesh.setupBuffers();
    return mesh;
}

// SPHERE (UV sphere with latitude/longitude subdivisions)
Mesh createSphere(float radius = 1.0f, int segments = 32, int rings = 16) {
    Mesh mesh;
    
    // Generate vertices
    for (int ring = 0; ring <= rings; ring++) {
        float phi = M_PI * float(ring) / float(rings);  // 0 to PI (top to bottom)
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * M_PI * float(seg) / float(segments);  // 0 to 2*PI (around)
            
            // Spherical to Cartesian coordinates
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            
            // Position
            mesh.vertices.push_back(x);
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(z);
            
            // Normal (for sphere, normalized position = normal)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            mesh.vertices.push_back(normal.x);
            mesh.vertices.push_back(normal.y);
            mesh.vertices.push_back(normal.z);
        }
    }
    
    // Generate indices
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    mesh.setupBuffers();
    return mesh;
}

// CYLINDER (circular cross-section along Y axis)
Mesh createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32) {
    Mesh mesh;
    float halfHeight = height / 2.0f;
    
    // Bottom cap center
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(-halfHeight);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(-1.0f);
    mesh.vertices.push_back(0.0f);
    
    // Top cap center
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(halfHeight);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(1.0f);
    mesh.vertices.push_back(0.0f);
    
    // Generate side vertices (duplicate for different normals)
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // Bottom ring (for bottom cap)
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(-halfHeight);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(0.0f);
        mesh.vertices.push_back(-1.0f);
        mesh.vertices.push_back(0.0f);
        
        // Top ring (for top cap)
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(halfHeight);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(0.0f);
        mesh.vertices.push_back(1.0f);
        mesh.vertices.push_back(0.0f);
        
        // Bottom ring (for side)
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(-halfHeight);
        mesh.vertices.push_back(z);
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
        mesh.vertices.push_back(normal.x);
        mesh.vertices.push_back(normal.y);
        mesh.vertices.push_back(normal.z);
        
        // Top ring (for side)
        mesh.vertices.push_back(x);
        mesh.vertices.push_back(halfHeight);
        mesh.vertices.push_back(z);
        mesh.vertices.push_back(normal.x);
        mesh.vertices.push_back(normal.y);
        mesh.vertices.push_back(normal.z);
    }
    
    // Bottom cap indices
    for (int i = 0; i < segments; i++) {
        mesh.indices.push_back(0);
        mesh.indices.push_back(2 + i * 4);
        mesh.indices.push_back(2 + (i + 1) * 4);
    }
    
    // Top cap indices
    for (int i = 0; i < segments; i++) {
        mesh.indices.push_back(1);
        mesh.indices.push_back(3 + (i + 1) * 4);
        mesh.indices.push_back(3 + i * 4);
    }
    
    // Side indices
    for (int i = 0; i < segments; i++) {
        int bottom = 4 + i * 4;
        int top = 5 + i * 4;
        int nextBottom = 4 + (i + 1) * 4;
        int nextTop = 5 + (i + 1) * 4;
        
        mesh.indices.push_back(bottom);
        mesh.indices.push_back(top);
        mesh.indices.push_back(nextBottom);
        
        mesh.indices.push_back(nextBottom);
        mesh.indices.push_back(top);
        mesh.indices.push_back(nextTop);
    }
    
    mesh.setupBuffers();
    return mesh;
}

// Shader compilation (same as before)
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

/* ============================================================================
   MAIN PROGRAM - EASY SCENE SETUP EXAMPLE
   ============================================================================ */

int main() {
    // Initialize GLFW and OpenGL
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1200, 800, "3D Primitive Shapes", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);
    
    GLuint shaderProgram = createShaderProgram();
    
    /* ========================================================================
       CREATE PRIMITIVE SHAPES
       
       Each mesh is created once and can be used by multiple objects
       ======================================================================== */
    
    Mesh planeMesh = createPlane(10.0f, 10.0f, 20, 20);
    Mesh cubeMesh = createCube(1.0f);
    Mesh sphereMesh = createSphere(1.0f, 32, 16);
    Mesh cylinderMesh = createCylinder(1.0f, 2.0f, 32);
    
    /* ========================================================================
       CREATE 3D OBJECTS
       
       Easy object creation - just specify mesh, position, and color!
       ======================================================================== */
    
    std::vector<Object3D> objects;
    
    // Ground plane (white)
    Object3D ground(&planeMesh, glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.9f, 0.9f, 0.9f));
    objects.push_back(ground);
    
    // Red cube on the left
    Object3D cube1(&cubeMesh, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.2f, 0.2f));
    cube1.scale = glm::vec3(1.5f);  // Make it bigger
    objects.push_back(cube1);
    
    // Green sphere in the center
    Object3D sphere1(&sphereMesh, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.2f, 1.0f, 0.2f));
    sphere1.scale = glm::vec3(1.2f);
    objects.push_back(sphere1);
    
    // Blue cylinder on the right
    Object3D cylinder1(&cylinderMesh, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.2f, 0.4f, 1.0f));
    objects.push_back(cylinder1);
    
    // Small yellow cube above
    Object3D cube2(&cubeMesh, glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(1.0f, 1.0f, 0.2f));
    cube2.scale = glm::vec3(0.7f);
    objects.push_back(cube2);
    
    // Back row - different colored spheres
    Object3D sphere2(&sphereMesh, glm::vec3(-2.0f, 0.0f, -3.0f), glm::vec3(1.0f, 0.5f, 0.0f));
    objects.push_back(sphere2);
    
    Object3D sphere3(&sphereMesh, glm::vec3(2.0f, 0.0f, -3.0f), glm::vec3(0.5f, 0.0f, 1.0f));
    objects.push_back(sphere3);
    
    /* ========================================================================
       RENDER LOOP
       ======================================================================== */
    
    while (!glfwWindowShouldClose(window)) {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // Get window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        // Camera setup - orbiting camera
        float camX = sin(glfwGetTime() * 0.3f) * 10.0f;
        float camZ = cos(glfwGetTime() * 0.3f) * 10.0f;
        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, 4.0f, camZ),  // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),  // Look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
        );
        
        float aspectRatio = (height > 0) ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        
        // Set view and projection (same for all objects)
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Lighting
        glm::vec3 lightPos(5.0f, 8.0f, 5.0f);
        glm::vec3 viewPos(camX, 4.0f, camZ);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        
        // Animate some objects
        objects[1].rotation.y = glfwGetTime() * 30.0f;  // Rotate red cube
        objects[2].position.y = sin(glfwGetTime() * 2.0f) * 0.5f;  // Bounce green sphere
        objects[4].rotation.x = glfwGetTime() * 45.0f;  // Rotate yellow cube
        objects[4].rotation.z = glfwGetTime() * 60.0f;
        
        /* ====================================================================
           DRAW ALL OBJECTS
           
           For each object: set its model matrix and color, then draw its mesh
           ==================================================================== */
        
        for (Object3D& obj : objects) {
            // Set model matrix for this object
            glm::mat4 model = obj.getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            
            // Set color for this object
            glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(obj.color));
            
            // Draw the mesh
            obj.mesh->draw();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    planeMesh.cleanup();
    cubeMesh.cleanup();
    sphereMesh.cleanup();
    cylinderMesh.cleanup();
    
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

/* ============================================================================
   USAGE GUIDE:
   
   1. CREATE A MESH (do this once):
      Mesh sphereMesh = createSphere(1.0f, 32, 16);
   
   2. CREATE OBJECTS FROM THE MESH:
      Object3D ball(&sphereMesh, glm::vec3(x, y, z), glm::vec3(r, g, b));
   
   3. MODIFY OBJECT PROPERTIES:
      ball.position = glm::vec3(5.0f, 2.0f, 0.0f);
      ball.rotation = glm::vec3(45.0f, 0.0f, 30.0f);
      ball.scale = glm::vec3(2.0f);  // Make it twice as big
      ball.color = glm::vec3(1.0f, 0.0f, 0.0f);  // Red
   
   4. IN RENDER LOOP:
      for (Object3D& obj : objects) {
          glm::mat4 model = obj.getModelMatrix();
          glUniformMatrix4fv(..., model);
          glUniform3fv(..., obj.color);
          obj.mesh->draw();
      }
   
   AVAILABLE SHAPES:
   - createPlane(width, depth, subdivisionsW, subdivisionsD)
   - createCube(size)
   - createSphere(radius, segments, rings)
   - createCylinder(radius, height, segments)
   
   You can easily add more shapes following the same pattern!
   ============================================================================ */

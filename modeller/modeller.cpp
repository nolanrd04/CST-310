#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

/* ============================================================================
   ULTRA-SIMPLE 3D SHAPE FRAMEWORK
   
   This framework provides simple functions to create shapes:
   - makeCube()
   - makeSphere()
   - makeCylinder()
   - makePlane()
   - makeRectangle()
   
   Just call these functions with position, rotation, color, and scale!
   ============================================================================ */

// Vertex Shader
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

// Fragment Shader
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
   MESH AND OBJECT STRUCTURES
   ============================================================================ */

struct Mesh {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    
    Mesh() : VAO(0), VBO(0), EBO(0) {}
    
    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

struct Object3D {
    Mesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
    
    Object3D(Mesh* m, glm::vec3 pos, glm::vec3 rot, glm::vec3 scl, glm::vec3 col)
        : mesh(m), position(pos), rotation(rot), scale(scl), color(col) {}
    
    glm::mat4 getModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};

/* ============================================================================
   GLOBAL STORAGE
   
   Stores all meshes and objects in the scene
   ============================================================================ */

std::vector<Mesh*> g_meshes;      // All created meshes (for cleanup)
std::vector<Object3D> g_objects;  // All objects in the scene

/* ============================================================================
   MESH GENERATION FUNCTIONS (Internal)
   ============================================================================ */

Mesh* generateCubeMesh() {
    Mesh* mesh = new Mesh();
    float s = 0.5f;  // Half size
    
    float cubeVertices[] = {
        // Front face
        -s, -s,  s,  0.0f, 0.0f, 1.0f,
         s, -s,  s,  0.0f, 0.0f, 1.0f,
         s,  s,  s,  0.0f, 0.0f, 1.0f,
        -s,  s,  s,  0.0f, 0.0f, 1.0f,
        // Back face
         s, -s, -s,  0.0f, 0.0f, -1.0f,
        -s, -s, -s,  0.0f, 0.0f, -1.0f,
        -s,  s, -s,  0.0f, 0.0f, -1.0f,
         s,  s, -s,  0.0f, 0.0f, -1.0f,
        // Left face
        -s, -s, -s,  -1.0f, 0.0f, 0.0f,
        -s, -s,  s,  -1.0f, 0.0f, 0.0f,
        -s,  s,  s,  -1.0f, 0.0f, 0.0f,
        -s,  s, -s,  -1.0f, 0.0f, 0.0f,
        // Right face
         s, -s,  s,  1.0f, 0.0f, 0.0f,
         s, -s, -s,  1.0f, 0.0f, 0.0f,
         s,  s, -s,  1.0f, 0.0f, 0.0f,
         s,  s,  s,  1.0f, 0.0f, 0.0f,
        // Top face
        -s,  s,  s,  0.0f, 1.0f, 0.0f,
         s,  s,  s,  0.0f, 1.0f, 0.0f,
         s,  s, -s,  0.0f, 1.0f, 0.0f,
        -s,  s, -s,  0.0f, 1.0f, 0.0f,
        // Bottom face
        -s, -s, -s,  0.0f, -1.0f, 0.0f,
         s, -s, -s,  0.0f, -1.0f, 0.0f,
         s, -s,  s,  0.0f, -1.0f, 0.0f,
        -s, -s,  s,  0.0f, -1.0f, 0.0f
    };
    
    for (int i = 0; i < 144; i++) {
        mesh->vertices.push_back(cubeVertices[i]);
    }
    
    for (int i = 0; i < 6; i++) {
        int offset = i * 4;
        mesh->indices.push_back(offset + 0);
        mesh->indices.push_back(offset + 1);
        mesh->indices.push_back(offset + 2);
        mesh->indices.push_back(offset + 0);
        mesh->indices.push_back(offset + 2);
        mesh->indices.push_back(offset + 3);
    }
    
    mesh->setupBuffers();
    g_meshes.push_back(mesh);
    return mesh;
}

Mesh* generateSphereMesh(int segments = 32, int rings = 16) {
    Mesh* mesh = new Mesh();
    float radius = 0.5f;
    
    for (int ring = 0; ring <= rings; ring++) {
        float phi = M_PI * float(ring) / float(rings);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * M_PI * float(seg) / float(segments);
            
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            
            mesh->vertices.push_back(x);
            mesh->vertices.push_back(y);
            mesh->vertices.push_back(z);
            
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            mesh->vertices.push_back(normal.x);
            mesh->vertices.push_back(normal.y);
            mesh->vertices.push_back(normal.z);
        }
    }
    
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            mesh->indices.push_back(current);
            mesh->indices.push_back(next);
            mesh->indices.push_back(current + 1);
            
            mesh->indices.push_back(current + 1);
            mesh->indices.push_back(next);
            mesh->indices.push_back(next + 1);
        }
    }
    
    mesh->setupBuffers();
    g_meshes.push_back(mesh);
    return mesh;
}

Mesh* generateCylinderMesh(int segments = 32) {
    Mesh* mesh = new Mesh();
    float radius = 0.5f;
    float halfHeight = 0.5f;
    
    // Bottom center
    mesh->vertices.push_back(0.0f);
    mesh->vertices.push_back(-halfHeight);
    mesh->vertices.push_back(0.0f);
    mesh->vertices.push_back(0.0f);
    mesh->vertices.push_back(-1.0f);
    mesh->vertices.push_back(0.0f);
    
    // Top center
    mesh->vertices.push_back(0.0f);
    mesh->vertices.push_back(halfHeight);
    mesh->vertices.push_back(0.0f);
    mesh->vertices.push_back(0.0f);
    mesh->vertices.push_back(1.0f);
    mesh->vertices.push_back(0.0f);
    
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // Bottom ring (for bottom cap)
        mesh->vertices.push_back(x);
        mesh->vertices.push_back(-halfHeight);
        mesh->vertices.push_back(z);
        mesh->vertices.push_back(0.0f);
        mesh->vertices.push_back(-1.0f);
        mesh->vertices.push_back(0.0f);
        
        // Top ring (for top cap)
        mesh->vertices.push_back(x);
        mesh->vertices.push_back(halfHeight);
        mesh->vertices.push_back(z);
        mesh->vertices.push_back(0.0f);
        mesh->vertices.push_back(1.0f);
        mesh->vertices.push_back(0.0f);
        
        // Bottom ring (for side)
        mesh->vertices.push_back(x);
        mesh->vertices.push_back(-halfHeight);
        mesh->vertices.push_back(z);
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
        mesh->vertices.push_back(normal.x);
        mesh->vertices.push_back(normal.y);
        mesh->vertices.push_back(normal.z);
        
        // Top ring (for side)
        mesh->vertices.push_back(x);
        mesh->vertices.push_back(halfHeight);
        mesh->vertices.push_back(z);
        mesh->vertices.push_back(normal.x);
        mesh->vertices.push_back(normal.y);
        mesh->vertices.push_back(normal.z);
    }
    
    // Bottom cap
    for (int i = 0; i < segments; i++) {
        mesh->indices.push_back(0);
        mesh->indices.push_back(2 + i * 4);
        mesh->indices.push_back(2 + (i + 1) * 4);
    }
    
    // Top cap
    for (int i = 0; i < segments; i++) {
        mesh->indices.push_back(1);
        mesh->indices.push_back(3 + (i + 1) * 4);
        mesh->indices.push_back(3 + i * 4);
    }
    
    // Side
    for (int i = 0; i < segments; i++) {
        int bottom = 4 + i * 4;
        int top = 5 + i * 4;
        int nextBottom = 4 + (i + 1) * 4;
        int nextTop = 5 + (i + 1) * 4;
        
        mesh->indices.push_back(bottom);
        mesh->indices.push_back(top);
        mesh->indices.push_back(nextBottom);
        
        mesh->indices.push_back(nextBottom);
        mesh->indices.push_back(top);
        mesh->indices.push_back(nextTop);
    }
    
    mesh->setupBuffers();
    g_meshes.push_back(mesh);
    return mesh;
}

Mesh* generatePlaneMesh(int subdivisionsW = 10, int subdivisionsD = 10) {
    Mesh* mesh = new Mesh();
    float width = 1.0f;
    float depth = 1.0f;
    
    for (int z = 0; z <= subdivisionsD; z++) {
        for (int x = 0; x <= subdivisionsW; x++) {
            float xPos = (float)x / subdivisionsW * width - width / 2.0f;
            float zPos = (float)z / subdivisionsD * depth - depth / 2.0f;
            
            mesh->vertices.push_back(xPos);
            mesh->vertices.push_back(0.0f);
            mesh->vertices.push_back(zPos);
            
            mesh->vertices.push_back(0.0f);
            mesh->vertices.push_back(1.0f);
            mesh->vertices.push_back(0.0f);
        }
    }
    
    for (int z = 0; z < subdivisionsD; z++) {
        for (int x = 0; x < subdivisionsW; x++) {
            int topLeft = z * (subdivisionsW + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (subdivisionsW + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            mesh->indices.push_back(topLeft);
            mesh->indices.push_back(bottomLeft);
            mesh->indices.push_back(topRight);
            
            mesh->indices.push_back(topRight);
            mesh->indices.push_back(bottomLeft);
            mesh->indices.push_back(bottomRight);
        }
    }
    
    mesh->setupBuffers();
    g_meshes.push_back(mesh);
    return mesh;
}

Mesh* generateRectangleMesh() {
    Mesh* mesh = new Mesh();
    float w = 0.5f;  // Half width
    float h = 0.5f;  // Half height
    
    // Simple rectangle in XY plane (like a wall or billboard)
    float rectVertices[] = {
        // Position           Normal
        -w, -h, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom left
         w, -h, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom right
         w,  h, 0.0f,  0.0f, 0.0f, 1.0f,  // Top right
        -w,  h, 0.0f,  0.0f, 0.0f, 1.0f   // Top left
    };
    
    for (int i = 0; i < 24; i++) {
        mesh->vertices.push_back(rectVertices[i]);
    }
    
    // Two triangles
    unsigned int rectIndices[] = {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };
    
    for (int i = 0; i < 6; i++) {
        mesh->indices.push_back(rectIndices[i]);
    }
    
    mesh->setupBuffers();
    g_meshes.push_back(mesh);
    return mesh;
}

/* ============================================================================
   EASY-TO-USE SHAPE CREATION FUNCTIONS
   
   These are the main functions you'll use to create objects!
   ============================================================================ */

/* ----------------------------------------------------------------------------
   makeCube - Create a cube and add it to the scene
   
   Arguments (all optional with defaults):
   - x, y, z:        Position coordinates (default: 0, 0, 0)
   - rotX, rotY, rotZ: Rotation in degrees (default: 0, 0, 0)
   - scaleX, scaleY, scaleZ: Scale factors (default: 1, 1, 1)
                             1.0 = normal size, 2.0 = double size, etc.
   - r, g, b:        Color values from 0.0 to 1.0 (default: 0.7, 0.7, 0.7 = grey)
   
   Example usage:
   makeCube();  // Grey cube at origin
   makeCube(0, 5, 0);  // Grey cube at position (0, 5, 0)
   makeCube(2, 0, 0, 0, 45, 0);  // Cube at (2,0,0) rotated 45° around Y axis
   makeCube(0, 0, 0, 0, 0, 0, 2, 1, 1);  // Wide cube (stretched along X)
   makeCube(1, 2, 3, 0, 0, 0, 1, 1, 1, 1, 0, 0);  // Red cube
   ---------------------------------------------------------------------------- */
void makeCube(float x = 0.0f, float y = 0.0f, float z = 0.0f,
              float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
              float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f,
              float r = 0.7f, float g = 0.7f, float b = 0.7f) {
    
    static Mesh* cubeMesh = nullptr;
    if (!cubeMesh) {
        cubeMesh = generateCubeMesh();
    }
    
    Object3D obj(cubeMesh, 
                 glm::vec3(x, y, z),                    // Position
                 glm::vec3(rotX, rotY, rotZ),           // Rotation
                 glm::vec3(scaleX, scaleY, scaleZ),     // Scale
                 glm::vec3(r, g, b));                   // Color
    
    g_objects.push_back(obj);
}

/* ----------------------------------------------------------------------------
   makeSphere - Create a sphere and add it to the scene
   
   Arguments (all optional with defaults):
   - x, y, z:        Position coordinates (default: 0, 0, 0)
   - rotX, rotY, rotZ: Rotation in degrees (default: 0, 0, 0)
                       Note: Rotation doesn't visually affect a uniform sphere
   - scaleX, scaleY, scaleZ: Scale factors (default: 1, 1, 1)
                             Use different values to create ellipsoids
   - r, g, b:        Color values from 0.0 to 1.0 (default: 0.7, 0.7, 0.7 = grey)
   
   Example usage:
   makeSphere();  // Grey sphere at origin
   makeSphere(5, 2, 0);  // Grey sphere at position (5, 2, 0)
   makeSphere(0, 0, 0, 0, 0, 0, 2, 2, 2);  // Large sphere (2x size)
   makeSphere(0, 3, 0, 0, 0, 0, 1, 2, 1);  // Tall ellipsoid
   makeSphere(-3, 1, 2, 0, 0, 0, 1, 1, 1, 0, 1, 0);  // Green sphere
   ---------------------------------------------------------------------------- */
void makeSphere(float x = 0.0f, float y = 0.0f, float z = 0.0f,
                float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
                float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f,
                float r = 0.7f, float g = 0.7f, float b = 0.7f) {
    
    static Mesh* sphereMesh = nullptr;
    if (!sphereMesh) {
        sphereMesh = generateSphereMesh(32, 16);
    }
    
    Object3D obj(sphereMesh,
                 glm::vec3(x, y, z),
                 glm::vec3(rotX, rotY, rotZ),
                 glm::vec3(scaleX, scaleY, scaleZ),
                 glm::vec3(r, g, b));
    
    g_objects.push_back(obj);
}

/* ----------------------------------------------------------------------------
   makeCylinder - Create a cylinder and add it to the scene
   
   Arguments (all optional with defaults):
   - x, y, z:        Position coordinates (default: 0, 0, 0)
   - rotX, rotY, rotZ: Rotation in degrees (default: 0, 0, 0)
                       Cylinder is aligned with Y axis by default
                       Use rotX or rotZ to tip it over
   - scaleX, scaleY, scaleZ: Scale factors (default: 1, 1, 1)
                             scaleX/scaleZ affect radius
                             scaleY affects height
   - r, g, b:        Color values from 0.0 to 1.0 (default: 0.7, 0.7, 0.7 = grey)
   
   Example usage:
   makeCylinder();  // Vertical grey cylinder at origin
   makeCylinder(2, 1, 0);  // Cylinder at position (2, 1, 0)
   makeCylinder(0, 0, 0, 90, 0, 0);  // Horizontal cylinder (rotated 90° around X)
   makeCylinder(0, 0, 0, 0, 0, 0, 1, 3, 1);  // Tall thin cylinder
   makeCylinder(0, 2, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1);  // Blue cylinder
   ---------------------------------------------------------------------------- */
void makeCylinder(float x = 0.0f, float y = 0.0f, float z = 0.0f,
                  float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
                  float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f,
                  float r = 0.7f, float g = 0.7f, float b = 0.7f) {
    
    static Mesh* cylinderMesh = nullptr;
    if (!cylinderMesh) {
        cylinderMesh = generateCylinderMesh(32);
    }
    
    Object3D obj(cylinderMesh,
                 glm::vec3(x, y, z),
                 glm::vec3(rotX, rotY, rotZ),
                 glm::vec3(scaleX, scaleY, scaleZ),
                 glm::vec3(r, g, b));
    
    g_objects.push_back(obj);
}

/* ----------------------------------------------------------------------------
   makePlane - Create a flat plane and add it to the scene
   
   Arguments (all optional with defaults):
   - x, y, z:        Position coordinates (default: 0, 0, 0)
   - rotX, rotY, rotZ: Rotation in degrees (default: 0, 0, 0)
                       Plane faces up (Y axis) by default
                       Use rotX to tilt it (90° makes it vertical)
   - scaleX, scaleY, scaleZ: Scale factors (default: 1, 1, 1)
                             scaleX affects width, scaleZ affects depth
                             scaleY has no visual effect on a flat plane
   - r, g, b:        Color values from 0.0 to 1.0 (default: 0.7, 0.7, 0.7 = grey)
   
   Example usage:
   makePlane();  // Grey horizontal plane at origin
   makePlane(0, -2, 0, 0, 0, 0, 10, 1, 10);  // Large ground plane
   makePlane(0, 0, -5, 90, 0, 0);  // Vertical wall (back of scene)
   makePlane(0, 0, 0, 0, 0, 0, 5, 1, 3, 0.3, 0.8, 0.3);  // Green grass
   ---------------------------------------------------------------------------- */
void makePlane(float x = 0.0f, float y = 0.0f, float z = 0.0f,
               float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
               float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f,
               float r = 0.7f, float g = 0.7f, float b = 0.7f) {
    
    static Mesh* planeMesh = nullptr;
    if (!planeMesh) {
        planeMesh = generatePlaneMesh(10, 10);
    }
    
    Object3D obj(planeMesh,
                 glm::vec3(x, y, z),
                 glm::vec3(rotX, rotY, rotZ),
                 glm::vec3(scaleX, scaleY, scaleZ),
                 glm::vec3(r, g, b));
    
    g_objects.push_back(obj);
}

/* ----------------------------------------------------------------------------
   makeRectangle - Create a flat rectangle and add it to the scene
   
   Arguments (all optional with defaults):
   - x, y, z:        Position coordinates (default: 0, 0, 0)
   - rotX, rotY, rotZ: Rotation in degrees (default: 0, 0, 0)
                       Rectangle faces forward (Z axis) by default
                       Great for billboards, walls, or UI elements
   - scaleX, scaleY, scaleZ: Scale factors (default: 1, 1, 1)
                             scaleX affects width, scaleY affects height
                             scaleZ has no visual effect
   - r, g, b:        Color values from 0.0 to 1.0 (default: 0.7, 0.7, 0.7 = grey)
   
   Example usage:
   makeRectangle();  // Grey rectangle facing camera
   makeRectangle(0, 2, 0, 0, 0, 0, 3, 2);  // Wide rectangular panel
   makeRectangle(0, 0, -5, 0, 180, 0);  // Rectangle facing away
   makeRectangle(-2, 1, 0, 0, 90, 0, 1, 2, 1, 1, 1, 0);  // Yellow vertical rect
   ---------------------------------------------------------------------------- */
void makeRectangle(float x = 0.0f, float y = 0.0f, float z = 0.0f,
                   float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
                   float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f,
                   float r = 0.7f, float g = 0.7f, float b = 0.7f) {
    
    static Mesh* rectangleMesh = nullptr;
    if (!rectangleMesh) {
        rectangleMesh = generateRectangleMesh();
    }
    
    Object3D obj(rectangleMesh,
                 glm::vec3(x, y, z),
                 glm::vec3(rotX, rotY, rotZ),
                 glm::vec3(scaleX, scaleY, scaleZ),
                 glm::vec3(r, g, b));
    
    g_objects.push_back(obj);
}

/* ============================================================================
   UTILITY FUNCTIONS
   ============================================================================ */

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
   MAIN PROGRAM - DEMONSTRATION
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
    
    GLFWwindow* window = glfwCreateWindow(1200, 800, "Easy 3D Shapes", NULL, NULL);
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
       CREATE YOUR SCENE HERE!
       
       Just call the make functions with your desired parameters
       ======================================================================== */
    
    // Ground plane - large, white, positioned below origin
    makePlane(0, -2, 0,           // Position (x, y, z)
              0, 0, 0,             // Rotation (rotX, rotY, rotZ)
              10, 1, 10,           // Scale (scaleX, scaleY, scaleZ)
              0.9, 0.9, 0.9);      // Color (r, g, b) - white
    
    // Red cube on the left
    makeCube(-3, 0, 0,             // Position
             0, 0, 0,              // Rotation
             1.5, 1.5, 1.5,        // Scale (1.5x bigger)
             1.0, 0.2, 0.2);       // Color - red
    
    // Green sphere in the center
    makeSphere(0, 0, 0,            // Position
               0, 0, 0,            // Rotation
               1.2, 1.2, 1.2,      // Scale
               0.2, 1.0, 0.2);     // Color - green
    
    // Blue cylinder on the right
    makeCylinder(3, 0, 0,          // Position
                 0, 0, 0,          // Rotation
                 1, 1, 1,          // Scale
                 0.2, 0.4, 1.0);   // Color - blue
    
    // Small yellow cube floating above
    makeCube(0, 2.5, 0,            // Position
             0, 0, 0,              // Rotation
             0.7, 0.7, 0.7,        // Scale (smaller)
             1.0, 1.0, 0.2);       // Color - yellow
    
    // Orange sphere in back left
    makeSphere(-2, 0, -3,          // Position
               0, 0, 0,            // Rotation
               1, 1, 1,            // Scale
               1.0, 0.5, 0.0);     // Color - orange
    
    // Purple sphere in back right
    makeSphere(2, 0, -3,           // Position
               0, 0, 0,            // Rotation
               1, 1, 1,            // Scale
               0.5, 0.0, 1.0);     // Color - purple
    
    // Pink rectangle (like a billboard)
    makeRectangle(-4, 1, -2,       // Position
                  0, 45, 0,        // Rotation (turned 45° toward camera)
                  1.5, 2, 1,       // Scale (wide and tall)
                  1.0, 0.5, 0.8);  // Color - pink
    
    // Horizontal cylinder (like a log)
    makeCylinder(4, -1, -1,        // Position
                 0, 0, 90,         // Rotation (90° around Z = horizontal)
                 0.3, 2, 0.3,      // Scale (long and thin)
                 0.6, 0.4, 0.2);   // Color - brown
    
    /* ========================================================================
       RENDER LOOP
       ======================================================================== */
    
    float time = 0.0f;
    
    while (!glfwWindowShouldClose(window)) {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        time = glfwGetTime();
        
        // Get window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        // Camera setup - orbiting camera
        float camX = sin(time * 0.3f) * 10.0f;
        float camZ = cos(time * 0.3f) * 10.0f;
        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, 4.0f, camZ),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        
        float aspectRatio = (height > 0) ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        
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
        g_objects[1].rotation.y = time * 30.0f;           // Rotate red cube
        g_objects[2].position.y = sin(time * 2.0f) * 0.5f; // Bounce green sphere
        g_objects[4].rotation.x = time * 45.0f;            // Rotate yellow cube
        g_objects[4].rotation.z = time * 60.0f;
        
        // Draw all objects
        for (Object3D& obj : g_objects) {
            glm::mat4 model = obj.getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(obj.color));
            obj.mesh->draw();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    for (Mesh* mesh : g_meshes) {
        mesh->cleanup();
        delete mesh;
    }
    
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

/* ============================================================================
   QUICK REFERENCE GUIDE
   ============================================================================
   
   FUNCTION SIGNATURES:
   
   makeCube(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
   makeSphere(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
   makeCylinder(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
   makePlane(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
   makeRectangle(x, y, z, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, r, g, b)
   
   PARAMETER ORDER (same for all shapes):
   1-3:   Position (x, y, z)
   4-6:   Rotation in degrees (rotX, rotY, rotZ)
   7-9:   Scale (scaleX, scaleY, scaleZ)
   10-12: Color (r, g, b) - values from 0.0 to 1.0
   
   COMMON COLORS:
   Red:     1.0, 0.0, 0.0
   Green:   0.0, 1.0, 0.0
   Blue:    0.0, 0.0, 1.0
   Yellow:  1.0, 1.0, 0.0
   Cyan:    0.0, 1.0, 1.0
   Magenta: 1.0, 0.0, 1.0
   White:   1.0, 1.0, 1.0
   Black:   0.0, 0.0, 0.0
   Grey:    0.5, 0.5, 0.5
   
   EXAMPLES:
   
   // Minimum (all defaults):
   makeCube();
   
   // Position only:
   makeSphere(5, 2, 0);
   
   // Position + rotation:
   makeCylinder(0, 0, 0, 90, 0, 0);
   
   // Position + rotation + scale:
   makeCube(1, 1, 1, 0, 45, 0, 2, 1, 1);
   
   // All parameters:
   makeSphere(2, 3, 4, 0, 0, 0, 1.5, 1.5, 1.5, 1.0, 0.5, 0.0);
   
   ============================================================================ */

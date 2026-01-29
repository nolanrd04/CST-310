#include <GL/glew.h>        // OpenGL Extension Wrangler - loads modern OpenGL functions
#include <GLFW/glfw3.h>      // GLFW - handles window creation and input
#include <glm/glm.hpp>       // GLM - OpenGL Mathematics library for vectors/matrices
#include <glm/gtc/matrix_transform.hpp>  // Matrix transformation functions (translate, rotate, perspective)
#include <glm/gtc/type_ptr.hpp>          // Converts GLM types to raw pointers for OpenGL
#include <iostream>
#include <cmath>

/* ============================================================================
   SHADER PROGRAMS
   
   Shaders run on the GPU and process vertices and fragments (pixels).
   They're written in GLSL (OpenGL Shading Language).
   
   The pipeline: Vertices → Vertex Shader → Rasterization → Fragment Shader → Screen
   ============================================================================ */

// VERTEX SHADER
// Runs once per vertex. Transforms 3D positions to screen coordinates.
const char* vertexShaderSource = R"(
#version 330 core

// INPUT: Vertex data from our C++ program
layout (location = 0) in vec3 aPos;      // Vertex position (x, y, z)
layout (location = 1) in vec3 aNormal;   // Surface normal vector (for lighting)

// OUTPUT: Data passed to fragment shader
out vec3 FragPos;   // Position in world space
out vec3 Normal;    // Normal vector in world space

// UNIFORMS: Constants that stay the same for all vertices in one draw call
uniform mat4 model;       // Transforms object space → world space
uniform mat4 view;        // Transforms world space → camera space
uniform mat4 projection;  // Transforms camera space → clip space (2D screen)

void main()
{
    // Transform vertex position to world space
    // vec4(aPos, 1.0) converts 3D position to homogeneous coordinates (needed for matrix math)
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal vector to world space
    // We use transpose(inverse(model)) because normals transform differently than positions
    // This ensures normals stay perpendicular to surfaces even with non-uniform scaling
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Final vertex position: object → world → camera → screen
    // This is what determines WHERE the vertex appears on screen
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// FRAGMENT SHADER
// Runs once per pixel. Calculates the final color using Phong lighting model.
const char* fragmentShaderSource = R"(
#version 330 core

// OUTPUT: Final pixel color
out vec4 FragColor;

// INPUT: Interpolated data from vertex shader
in vec3 FragPos;   // Position of this pixel in world space
in vec3 Normal;    // Normal vector at this pixel (interpolated from vertices)

// UNIFORMS: Lighting and material properties
uniform vec3 lightPos;     // Position of light source
uniform vec3 viewPos;      // Position of camera
uniform vec3 lightColor;   // Color/intensity of light
uniform vec3 objectColor;  // Base color of the curtain

void main()
{
    /* PHONG LIGHTING MODEL
       Combines three types of lighting to create realistic shading:
       1. Ambient - base illumination (simulates indirect light)
       2. Diffuse - directional light (brighter when surface faces light)
       3. Specular - shiny highlights (reflection of light source)
    */
    
    // 1. AMBIENT LIGHTING
    // Provides base illumination so objects aren't completely black in shadow
    float ambientStrength = 0.3;  // 30% of light color
    vec3 ambient = ambientStrength * lightColor;
    
    // 2. DIFFUSE LIGHTING (Lambertian reflection)
    // Surfaces facing the light are brighter
    vec3 norm = normalize(Normal);              // Ensure normal is unit length
    vec3 lightDir = normalize(lightPos - FragPos);  // Direction from surface to light
    
    // Dot product tells us how aligned the surface is with light direction
    // dot(norm, lightDir) = cos(angle between vectors)
    // = 1.0 when surface directly faces light
    // = 0.0 when light is perpendicular
    // max() clamps negative values (surface facing away) to 0
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 3. SPECULAR LIGHTING (Shiny highlights)
    // Creates bright spots where light reflects toward the camera
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);  // Direction from surface to camera
    
    // reflect() calculates light bounce direction
    // -lightDir because reflect expects incident direction (toward surface)
    vec3 reflectDir = reflect(-lightDir, norm);
    
    // How aligned is the reflection with the view direction?
    // pow(..., 32) creates a tight highlight (higher = shinier surface)
    // The exponent controls how focused the specular highlight is
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // FINAL COLOR: Combine all lighting components
    // Multiply by object color to tint the light
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);  // Alpha = 1.0 (fully opaque)
}
)";

/* ============================================================================
   SHADER COMPILATION
   
   Shaders are compiled at runtime (like a mini compiler inside your program)
   ============================================================================ */

GLuint compileShader(GLenum type, const char* source) {
    // Create a shader object on the GPU
    GLuint shader = glCreateShader(type);
    
    // Upload source code to GPU
    glShaderSource(shader, 1, &source, NULL);
    
    // Compile the shader
    glCompileShader(shader);
    
    // Error checking - did compilation succeed?
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

// Create a complete shader program (vertex + fragment shader linked together)
GLuint createShaderProgram() {
    // Compile both shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    // Create program and attach shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    // Link shaders together (connects vertex shader outputs to fragment shader inputs)
    glLinkProgram(shaderProgram);
    
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    
    // Clean up individual shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// Callback when window is resized - updates the viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Viewport defines what part of the window OpenGL renders to
    glViewport(0, 0, width, height);
}

/* ============================================================================
   MESH GENERATION
   
   Creates a grid of vertices for the curtain. More subdivisions = smoother lighting.
   
   MATH EXPLANATION:
   - We create a 2D grid in the XY plane (Z=0)
   - Grid has (subdivisionsW+1) × (subdivisionsH+1) vertices
   - Each quad is split into 2 triangles (6 indices per quad)
   
   Example 2×2 grid:
   
   (0,1)---(1,1)---(2,1)
     |   /   |   /   |
     | /     | /     |
   (0,0)---(1,0)---(2,0)
   
   This creates 9 vertices and 8 triangles (4 quads × 2 triangles each)
   ============================================================================ */

void generateCurtainMesh(float width, float height, int subdivisionsW, int subdivisionsH,
                         float* &vertices, unsigned int* &indices, int &vertexCount, int &indexCount) {
    
    // Calculate array sizes
    vertexCount = (subdivisionsW + 1) * (subdivisionsH + 1);
    indexCount = subdivisionsW * subdivisionsH * 6;  // 6 indices per quad (2 triangles)
    
    // Each vertex has 6 floats: position (x,y,z) + normal (nx,ny,nz)
    vertices = new float[vertexCount * 6];
    indices = new unsigned int[indexCount];
    
    // GENERATE VERTICES
    for (int y = 0; y <= subdivisionsH; y++) {
        for (int x = 0; x <= subdivisionsW; x++) {
            int index = (y * (subdivisionsW + 1) + x) * 6;
            
            // POSITION CALCULATION
            // Map grid coordinates (x, y) to world space
            // x/subdivisionsW gives us 0.0 to 1.0
            // Multiply by width, subtract half to center at origin
            vertices[index + 0] = (float)x / subdivisionsW * width - width / 2.0f;   // X
            vertices[index + 1] = (float)y / subdivisionsH * height - height / 2.0f; // Y
            vertices[index + 2] = 0.0f;  // Z (flat curtain in XY plane)
            
            // NORMAL VECTOR
            // All normals point toward viewer (positive Z direction)
            // This tells the shader which way the surface is facing
            vertices[index + 3] = 0.0f;  // Normal X
            vertices[index + 4] = 0.0f;  // Normal Y
            vertices[index + 5] = 1.0f;  // Normal Z (pointing at camera)
        }
    }
    
    // GENERATE INDICES
    // Indices define which vertices form triangles
    // We use an Element Buffer to reuse vertices (more efficient than duplicating)
    int idx = 0;
    for (int y = 0; y < subdivisionsH; y++) {
        for (int x = 0; x < subdivisionsW; x++) {
            // Calculate vertex indices for this quad
            // Grid layout means each vertex is at: row * (subdivisionsW+1) + column
            int topLeft = y * (subdivisionsW + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * (subdivisionsW + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            // First triangle (top-left, bottom-left, top-right)
            // Counter-clockwise winding for front-facing
            indices[idx++] = topLeft;
            indices[idx++] = bottomLeft;
            indices[idx++] = topRight;
            
            // Second triangle (top-right, bottom-left, bottom-right)
            indices[idx++] = topRight;
            indices[idx++] = bottomLeft;
            indices[idx++] = bottomRight;
        }
    }
}

/* ============================================================================
   MAIN PROGRAM
   
   Sets up OpenGL context, creates mesh, and runs the render loop
   ============================================================================ */

int main() {
    // INITIALIZATION
    
    // Initialize GLFW (window management library)
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Curtain with Lighting", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Register callback for window resizing
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initialize GLEW (loads OpenGL function pointers)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Enable depth testing (so closer objects appear in front)
    glEnable(GL_DEPTH_TEST);
    
    // Set initial viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // Create shader program
    GLuint shaderProgram = createShaderProgram();
    
    /* ========================================================================
       MESH SETUP
       
       Create vertex data and upload to GPU
       
       OpenGL uses three buffer objects:
       - VAO (Vertex Array Object): Stores the vertex attribute configuration
       - VBO (Vertex Buffer Object): Stores actual vertex data
       - EBO (Element Buffer Object): Stores indices for vertex reuse
       ======================================================================== */
    
    // Generate mesh data (CPU side)
    float* vertices;
    unsigned int* indices;
    int vertexCount, indexCount;
    generateCurtainMesh(2.0f, 2.25f, 20, 23, vertices, indices, vertexCount, indexCount);
    
    // Create buffer objects on GPU
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);  // VAO stores vertex attribute setup
    glGenBuffers(1, &VBO);        // VBO holds vertex data
    glGenBuffers(1, &EBO);        // EBO holds indices
    
    // Bind VAO first - it will remember all subsequent vertex attribute calls
    glBindVertexArray(VAO);
    
    // Upload vertex data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 6 * sizeof(float), vertices, GL_STATIC_DRAW);
    
    // Upload index data to GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    // CONFIGURE VERTEX ATTRIBUTES
    // Tell OpenGL how to interpret the vertex data
    
    // Attribute 0: Position (3 floats)
    // Parameters: (index, size, type, normalized, stride, offset)
    // Stride = 6*sizeof(float) because each vertex is 6 floats
    // Offset = 0 because position comes first
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Attribute 1: Normal (3 floats)
    // Offset = 3*sizeof(float) because normal comes after position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind VAO (good practice to prevent accidental modification)
    glBindVertexArray(0);
    
    // Clean up CPU memory (data is now on GPU)
    delete[] vertices;
    delete[] indices;
    
    /* ========================================================================
       RENDER LOOP
       
       Continuously draws frames until window is closed
       ======================================================================== */
    
    while (!glfwWindowShouldClose(window)) {
        // INPUT HANDLING
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // Update viewport for current window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        // CLEAR THE SCREEN
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);  // Dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // ACTIVATE SHADER PROGRAM
        glUseProgram(shaderProgram);
        
        /* ====================================================================
           TRANSFORMATION MATRICES
           
           These matrices transform our 3D curtain to 2D screen coordinates:
           
           1. MODEL matrix: Positions/rotates the curtain in world space
           2. VIEW matrix: Simulates camera position
           3. PROJECTION matrix: Creates perspective (distant = smaller)
           
           Combined: Vertex_screen = Projection × View × Model × Vertex_local
           ==================================================================== */
        
        // MODEL MATRIX: Rotate curtain slowly over time
        glm::mat4 model = glm::mat4(1.0f);  // Start with identity matrix
        // Rotate around Y axis (vertical) at 0.1 radians/second
        model = glm::rotate(model, (float)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // VIEW MATRIX: Position camera
        glm::mat4 view = glm::mat4(1.0f);
        // Move camera back 5 units (negative Z means "away from screen")
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
        
        // PROJECTION MATRIX: Create perspective
        // Parameters: (field of view, aspect ratio, near clip, far clip)
        // Field of view = 45° (wider = more "fisheye", narrower = more "telephoto")
        // Aspect ratio = width/height (prevents stretching)
        // Near/far clip = only render objects between 0.1 and 100 units from camera
        float aspectRatio = (height > 0) ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        
        // UPLOAD MATRICES TO SHADER
        // Uniforms are variables that stay constant for all vertices/fragments in a draw call
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // LIGHTING PROPERTIES
        glm::vec3 lightPos(3.0f, 3.0f, 3.0f);       // Light position (upper-right-front)
        glm::vec3 viewPos(0.0f, 0.0f, 5.0f);        // Camera position (matches view matrix)
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);     // White light
        glm::vec3 objectColor(0.6f, 0.6f, 0.65f);   // Grey curtain
        
        // Upload lighting data to shader
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(objectColor));
        
        // DRAW THE CURTAIN
        glBindVertexArray(VAO);  // Use our vertex configuration
        // Draw using indices (more efficient than GL_TRIANGLES without indices)
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        // SWAP BUFFERS (double buffering prevents flickering)
        // We draw to back buffer while front buffer is displayed, then swap
        glfwSwapBuffers(window);
        
        // Process window events (keyboard, mouse, etc.)
        glfwPollEvents();
    }
    
    // CLEANUP
    // Free GPU resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    // Terminate GLFW
    glfwTerminate();
    return 0;
}

/* ============================================================================
   SUMMARY OF HOW COMPONENTS REFERENCE EACH OTHER:
   
   1. CPU → GPU Data Flow:
      - generateCurtainMesh() creates vertex data (CPU)
      - VBO stores vertices on GPU
      - EBO stores indices on GPU
      - VAO remembers how to interpret VBO data
   
   2. Shader Pipeline:
      - Vertex shader reads from VAO (layout location 0/1)
      - Transforms vertices using model/view/projection matrices
      - Outputs FragPos and Normal to fragment shader
      - Fragment shader uses these + lighting uniforms to calculate color
   
   3. Transformation Flow:
      Object space (mesh) → Model → World space → View → Camera space 
      → Projection → Clip space → Screen space
   
   4. Lighting Calculation:
      - Normal vectors (from mesh) define surface orientation
      - Light position and fragment position determine light direction
      - Camera position determines specular highlights
      - Phong model combines ambient + diffuse + specular
   
   5. Render Loop:
      - Updates time-based rotation (model matrix)
      - Sets uniforms (matrices, lighting)
      - Binds VAO (vertex configuration)
      - Draws with indices
      - Swaps buffers to display result
   ============================================================================ */
// wave.cpp
// Build: g++ main.cpp -std=c++17 -O2 -lglfw -lGLEW -lGL -ldl -o mesh_demo

#include <iostream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Define the mathematical function
static float f(float x, float z)
{
    // A simple wavy surface:
    return std::sin(x) * std::cos(z);
}

// Minimal shader compilation helpers
static GLuint compileShader(GLenum type, const char* src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(sh, len, nullptr, log.data());
        std::cerr << "Shader compile error:\n" << log << "\n";
    }
    return sh;
}

static GLuint linkProgram(GLuint vs, GLuint fs)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(prog, len, nullptr, log.data());
        std::cerr << "Program link error:\n" << log << "\n";
    }

    // Once linked, detach/delete shaders
    glDetachShader(prog, vs);
    glDetachShader(prog, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// Generate a grid mesh with vertices and indices
// Build a regular grid in XZ, compute Y via f(x,z), then triangulate
static void buildGridMesh(
    int gridN,                 // number of vertices along one axis (>= 2)
    float size,                // total width/depth of grid in world units
    std::vector<float>& vtx,   // output vertex data
    std::vector<unsigned>& idx // output triangle indices
)
{
    vtx.clear();
    idx.clear();

    // Grid spans from -size / 2 to size / 2 in X and Z
    const float half = size * 0.5f;

    // Helper to compute y at (x,z)
    auto heightAt = [](float x, float z) {
        return f(x, z);
    };

    // Finite difference step for normal estimation
    const float eps = 0.01f;

    // Create vertices by gridN * gridN
    for (int j = 0; j < gridN; ++j)
    {
        for (int i = 0; i < gridN; ++i)
        {
            float u = (gridN == 1) ? 0.0f : (float)i / (gridN - 1);
            float v = (gridN == 1) ? 0.0f : (float)j / (gridN - 1);

            float x = -half + u * size;
            float z = -half + v * size;
            float y = heightAt(x, z);

            // Approximate normal
            float y_dx1 = heightAt(x + eps, z);
            float y_dx0 = heightAt(x - eps, z);
            float y_dz1 = heightAt(x, z + eps);
            float y_dz0 = heightAt(x, z - eps);

            float df_dx = (y_dx1 - y_dx0) / (2.0f * eps);
            float df_dz = (y_dz1 - y_dz0) / (2.0f * eps);

            glm::vec3 n(-df_dx, 1.0f, -df_dz);
            n = glm::normalize(n);

            // Store interleaved vertex attributes
            vtx.push_back(x);
            vtx.push_back(y);
            vtx.push_back(z);

            vtx.push_back(n.x);
            vtx.push_back(n.y);
            vtx.push_back(n.z);
        }
    }

    // Create indices
    for (int j = 0; j < gridN - 1; ++j)
    {
        for (int i = 0; i < gridN - 1; ++i)
        {
            unsigned i0 = (unsigned)(j * gridN + i);
            unsigned i1 = (unsigned)(j * gridN + (i + 1));
            unsigned i2 = (unsigned)((j + 1) * gridN + i);
            unsigned i3 = (unsigned)((j + 1) * gridN + (i + 1));

            // Triangle 1: i0, i2, i1
            idx.push_back(i0);
            idx.push_back(i2);
            idx.push_back(i1);

            // Triangle 2: i1, i2, i3
            idx.push_back(i1);
            idx.push_back(i2);
            idx.push_back(i3);
        }
    }
}

int main()
{
    // Initialize GLFW + Window
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(1000, 700, "Mesh from f(x,z) - GLEW", nullptr, nullptr);
    if (!win)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); // vsync

    // Initialize GLEW
    glewExperimental = GL_TRUE; // helps on core profile contexts
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        std::cerr << "GLEW init error: " << glewGetErrorString(glewErr) << "\n";
        return 1;
    }

    // Clear it once to avoid confusion
    glGetError();

    // Basic GL state
    glEnable(GL_DEPTH_TEST);

    // Build mesh CPU-side
    std::vector<float> vertices;
    std::vector<unsigned> indices;

    // gridN controls resolution
    const int gridN = 150;
    const float size = 10.0f;
    buildGridMesh(gridN, size, vertices, indices);

    // Upload mesh to GPU
    GLuint vao = 0, vbo = 0, ebo = 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // Index buffer
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Describe vertex layout to OpenGL
    const GLsizei stride = 6 * (GLsizei)sizeof(float);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // attribute location in shader
        3,                  // number of components (x,y,z)
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        stride,             // bytes between vertices
        (void*)0            // offset in buffer
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)(3 * sizeof(float))
    );

    glBindVertexArray(0);

    // Create minimal vertex shaders
    const char* vsSrc = R"GLSL(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec3 aNormal;

        uniform mat4 uMVP;
        uniform mat4 uModel;

        out vec3 vNormal;
        out vec3 vWorldPos;

        void main()
        {
            vec4 world = uModel * vec4(aPos, 1.0);
            vWorldPos = world.xyz;

            // Correct normal transform = inverse transpose of model matrix.
            // Here model is identity, but we keep it correct and explicit.
            mat3 normalMat = mat3(transpose(inverse(uModel)));
            vNormal = normalize(normalMat * aNormal);

            gl_Position = uMVP * vec4(aPos, 1.0);
        }
    )GLSL";

    // Simple fragment one-direction light shader
    const char* fsSrc = R"GLSL(
        #version 330 core
        in vec3 vNormal;
        in vec3 vWorldPos;

        out vec4 FragColor;

        uniform vec3 uLightDir; // direction *towards* the light (world space)

        void main()
        {
            vec3 N = normalize(vNormal);
            vec3 L = normalize(uLightDir);

            float ndotl = max(dot(N, L), 0.0);
            float ambient = 0.20;

            float shade = ambient + (1.0 - ambient) * ndotl;

            // Color based on height for a little visual interest
            float h = vWorldPos.y;
            vec3 base = mix(vec3(0.1, 0.3, 0.7), vec3(0.1, 0.8, 0.2), clamp(h * 0.5 + 0.5, 0.0, 1.0));

            FragColor = vec4(base * shade, 1.0);
        }
    )GLSL";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    GLuint prog = linkProgram(vs, fs);

    // Uniform locations
    GLint locMVP     = glGetUniformLocation(prog, "uMVP");
    GLint locModel   = glGetUniformLocation(prog, "uModel");
    GLint locLightDir= glGetUniformLocation(prog, "uLightDir");

    // Render loop
    while (!glfwWindowShouldClose(win))
    {
        glfwPollEvents();

        // ESC to quit
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(win, 1);

        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Simple camera, with mesh rotation
        float t = (float)glfwGetTime();
        glm::vec3 camPos = glm::vec3(std::cos(t * 0.35f) * 14.0f, 6.0f, std::sin(t * 0.35f) * 14.0f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up     = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 view = glm::lookAt(camPos, target, up);
        glm::mat4 proj = glm::perspective(glm::radians(55.0f), (h > 0 ? (float)w / (float)h : 1.0f), 0.1f, 100.0f);

        // Model matrix
        glm::mat4 model = glm::mat4(1.0f);

        // MVP used to position vertices on screen
        glm::mat4 mvp = proj * view * model;

        // Draw
        glUseProgram(prog);

        // Upload uniforms
        glUniformMatrix4fv(locMVP,   1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

        // Directional light placement
        glm::vec3 lightDir = glm::normalize(glm::vec3(0.6f, 1.0f, 0.4f));
        glUniform3fv(locLightDir, 1, glm::value_ptr(lightDir));

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);

        glfwSwapBuffers(win);
    }

    // GPU resources cleanup
    glDeleteProgram(prog);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}

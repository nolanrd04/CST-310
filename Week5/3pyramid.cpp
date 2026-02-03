// main.cpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

static int gViewIndex = 0; // 0,1,2 switchable with keys 1/2/3

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
        if (key == GLFW_KEY_1) gViewIndex = 0;
        if (key == GLFW_KEY_2) gViewIndex = 1;
        if (key == GLFW_KEY_3) gViewIndex = 2;
    }
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n";
    }
    return s;
}

static GLuint makeProgram(const char* vsSrc, const char* fsSrc) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, 1024, nullptr, log);
        std::cerr << "Program link error:\n" << log << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

int main() {
    // --- GLFW init ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(900, 600, "3 Pyramids - Camera Views (1/2/3)", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    glewInit();
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);

    // // --- GLAD init ---
    // if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    //     std::cerr << "Failed to init GLAD\n";
    //     return -1;
    // }

    glEnable(GL_DEPTH_TEST);

    // --- Shaders ---
    const char* vsSrc = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        uniform mat4 uMVP;
        void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
    )";

    const char* fsSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() { FragColor = vec4(uColor, 1.0); }
    )";

    GLuint prog = makeProgram(vsSrc, fsSrc);
    GLint uMVP   = glGetUniformLocation(prog, "uMVP");
    GLint uColor = glGetUniformLocation(prog, "uColor");

    // --- Pyramid mesh (one pyramid, drawn with triangles) ---
    // Base is a square on y=0; apex is at y=1
    glm::vec3 A(-0.5f, 0.0f, -0.5f);
    glm::vec3 B( 0.5f, 0.0f, -0.5f);
    glm::vec3 C( 0.5f, 0.0f,  0.5f);
    glm::vec3 D(-0.5f, 0.0f,  0.5f);
    glm::vec3 P( 0.0f, 1.0f,  0.0f);

    // 4 side faces + 2 triangles for the base = 6 triangles = 18 vertices
    float verts[] = {
        // sides (counter-clockwise)
        A.x,A.y,A.z,  B.x,B.y,B.z,  P.x,P.y,P.z,
        B.x,B.y,B.z,  C.x,C.y,C.z,  P.x,P.y,P.z,
        C.x,C.y,C.z,  D.x,D.y,D.z,  P.x,P.y,P.z,
        D.x,D.y,D.z,  A.x,A.y,A.z,  P.x,P.y,P.z,
        // base (two triangles, CCW when looking from below)
        A.x,A.y,A.z,  C.x,C.y,C.z,  B.x,B.y,B.z,
        A.x,A.y,A.z,  D.x,D.y,D.z,  C.x,C.y,C.z
    };

    GLuint vao=0, vbo=0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- Camera presets (positions + a common target) ---
    glm::vec3 target(0.0f, 0.4f, 0.0f); // aim roughly at the middle pyramid
    glm::vec3 camPos[3] = {
        glm::vec3(0.0f, 1.3f, 4.0f),   // View 1: front-ish
        glm::vec3(4.5f, 1.6f, 2.5f),   // View 2: from the right, angled
        glm::vec3(0.0f, 6.0f, 0.5f)    // View 3: top-down-ish
    };

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int w,h;
        glfwGetFramebufferSize(win, &w, &h);
        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;

        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projection + View (depends on gViewIndex)
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(camPos[gViewIndex], target, glm::vec3(0,1,0));

        // Model transforms for the 3 pyramids along x-axis
        glm::mat4 models[3] = {
            glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)),
            glm::translate(glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, 0.0f)),
            glm::translate(glm::mat4(1.0f), glm::vec3( 2.0f, 0.0f, 0.0f))
        };

        glm::vec3 colors[3] = {
            glm::vec3(0.9f, 0.3f, 0.3f),
            glm::vec3(0.3f, 0.9f, 0.4f),
            glm::vec3(0.3f, 0.5f, 0.95f)
        };

        glUseProgram(prog);
        glBindVertexArray(vao);

        for (int i = 0; i < 3; i++) {
            glm::mat4 mvp = proj * view * models[i];
            glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3fv(uColor, 1, glm::value_ptr(colors[i]));
            glDrawArrays(GL_TRIANGLES, 0, 18);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(win);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(prog);

    glfwTerminate();
    return 0;
}

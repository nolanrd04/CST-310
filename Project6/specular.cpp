#ifdef _WIN32
#include <windows.h> // Must be included before GL headers on Windows
#endif

#include <GL/freeglut.h>
#include <GL/glext.h> // Shader function pointer typedefs (PFNGLPROC)

#include <cctype>   // stdtolower for case-insensitive key handling
#include <cmath>    // stdsin, stdcos, stdsqrt
#include <cstdio>   // stdfprintf
#include <cstdlib>  // stdexit, EXIT_FAILURE
#include <vector>   // stdvector for shader compile/link logs
#include <atomic>   // std::atomic<float>, std::atomic<bool>
#include <string>   // std::string for std::stof
#include <thread>   // std::thread

// -----------------------------------------------------------------------------
// OpenGL Phong-style lighting demo
// 8 blue cubes in a 2x4 grid, each with a different material shininess
// Top row    2, 4, 8, 16
// Bottom row 32, 64, 128, 256
//
// This version uses a small GLSL 120 shader pair for per-fragment lighting
// (Phong-style), while still using immediate-mode geometry for simplicity
// -----------------------------------------------------------------------------

// Window dimensions (updated by reshape callback)
static int gWindowWidth  = 1200;
static int gWindowHeight = 700;

// Grid and cube layout
static const int   kRows       = 2;
static const int   kCols       = 4;
static const float kCubeSize   = 1.35f; // Uniform scale for each unit cube
static const float kColSpacing = 3.10f; // Horizontal spacing between cube centers
static const float kRowSpacing = 3.30f; // Vertical spacing between row centers
static const float kCubeYawDeg   = -24.0f; // Y-axis rotation applied to every cube
static const float kCubePitchDeg =  7.0f;  // X-axis rotation applied to every cube

// Shininess labels shown under cubes (exact values requested by prompt)
static const int kShininessLabels[kRows][kCols] = {
    { 2,  4,   8,   16 },
    { 32, 64, 128, 256 }
};

static const char* kLabelText[kRows][kCols] = {
    { "2", "4", "8", "16" },
    { "32", "64", "128", "256" }
};

// -----------------------------------------------------------------------------
// Camera and input state
// -----------------------------------------------------------------------------
//
// Controls
// - WASD move along horizontal plane
// - Q / E move down / up
// - Arrow keys look left/right/up/down
//
// Initial pose matches the previous static camera view
static float gCameraPos[3] = { 0.60f, 0.35f, 13.50f };
static float gCameraYawDeg   = -2.54f;
static float gCameraPitchDeg = -1.48f;

// Camera tuning (units/seconds and degrees/second)
static const float kCameraMoveSpeed    = 7.50f;
static const float kCameraLookSpeedDeg = 90.00f;

// Delta-time cap prevents large jumps if the app stalls for a frame
static const float kMaxDeltaTimeSeconds = 0.050f;

// Held-key state for smooth continuous movement
static bool gKeyDown[256] = { false };

// Arrow keys are reported via GLUT "special" callbacks
static bool gArrowLeftDown  = false;
static bool gArrowRightDown = false;
static bool gArrowUpDown    = false;
static bool gArrowDownDown  = false;

// Timer baseline used by idle() for frame-rate-independent camera updates
static int gPrevTimeMs = 0;

// ---------------------------------------------------------------------------
// Query cube state — shared between the background console thread and the
// OpenGL render thread. Negative sentinel (-1.0f) means no query yet.
// ---------------------------------------------------------------------------
static std::atomic<float> gQueryShininess(-1.0f);
static std::atomic<bool>  gInputThreadShouldExit(false);
static std::thread        gInputThread;

// -----------------------------------------------------------------------------
// Shared light/material constants
// -----------------------------------------------------------------------------

// Base material color from rgba(174, 87, 54, 255)
static const GLfloat kMatDiffuse[3]  = {
    174.0f / 255.0f,
    87.0f  / 255.0f,
    54.0f  / 255.0f
};

// Ambient ~= 20% of diffuse
static const GLfloat kMatAmbient[3]  = {
    // Original scale 020f
    kMatDiffuse[0] * 0.16f,
    kMatDiffuse[1] * 0.16f,
    kMatDiffuse[2] * 0.16f
};

// White specular for visible highlights on all cubes
static const GLfloat kMatSpecular[3] = { 1.0f, 1.0f, 1.0f };

// Light distance from each cube's target highlight point (front-face center)
// Kept as a "handful of units" away from the cube
static const GLfloat kPerCubeLightDistance = 4.00f;

// Explicit ambient/diffuse/specular light colors
// Original kLightAmbient { 012f, 012f, 012f }
static const GLfloat kLightAmbient[3]  = { 0.08f, 0.08f, 0.08f };
// Original kLightDiffuse { 095f, 095f, 095f }
static const GLfloat kLightDiffuse[3]  = { 0.88f, 0.88f, 0.88f };
static const GLfloat kLightSpecular[3] = { 1.00f, 1.00f, 1.00f };
// Small boost to make specular hotspots stand out more
// Original kSpecularBoost 100f
static const GLfloat kSpecularBoost = 0.5f;

// -----------------------------------------------------------------------------
// Minimal shader function-pointer loader (works with FreeGLUT's proc lookup)
// This avoids requiring external loader libraries like GLEW
// -----------------------------------------------------------------------------

static PFNGLCREATESHADERPROC       pglCreateShader       = nullptr;
static PFNGLSHADERSOURCEPROC       pglShaderSource       = nullptr;
static PFNGLCOMPILESHADERPROC      pglCompileShader      = nullptr;
static PFNGLGETSHADERIVPROC        pglGetShaderiv        = nullptr;
static PFNGLGETSHADERINFOLOGPROC   pglGetShaderInfoLog   = nullptr;
static PFNGLDELETESHADERPROC       pglDeleteShader       = nullptr;
static PFNGLCREATEPROGRAMPROC      pglCreateProgram      = nullptr;
static PFNGLATTACHSHADERPROC       pglAttachShader       = nullptr;
static PFNGLLINKPROGRAMPROC        pglLinkProgram        = nullptr;
static PFNGLGETPROGRAMIVPROC       pglGetProgramiv       = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC  pglGetProgramInfoLog  = nullptr;
static PFNGLUSEPROGRAMPROC         pglUseProgram         = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation = nullptr;
static PFNGLUNIFORM3FPROC          pglUniform3f          = nullptr;
static PFNGLUNIFORM1FPROC          pglUniform1f          = nullptr;
static PFNGLDETACHSHADERPROC       pglDetachShader       = nullptr;
static PFNGLDELETEPROGRAMPROC      pglDeleteProgram      = nullptr;

static GLuint gPhongProgram = 0;

// Uniform locations
static GLint gULightPosEye   = -1;
static GLint gULightAmbient  = -1;
static GLint gULightDiffuse  = -1;
static GLint gULightSpecular = -1;
static GLint gUMatAmbient    = -1;
static GLint gUMatDiffuse    = -1;
static GLint gUMatSpecular   = -1;
static GLint gUMatShininess  = -1;
static GLint gUSpecularBoost = -1;

// Keep labels unchanged but slightly relax the highest exponent values so the
// hotspot remains visible at this view distance
static float EffectiveShininessFromLabel(int labelValue) {
    if (labelValue >= 256) return 256.0f;
    if (labelValue >= 128) return 128.0f;
    return static_cast<float>(labelValue);
}

// Converts degrees to radians for camera angle math
static float DegreesToRadians(float deg) {
    return deg * 0.01745329251994329577f;
}

// Simple scalar clamp utility
static float ClampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

// Transforms a world-space point into eye space using the current view matrix
static void TransformPointByMatrix(const GLdouble m[16], float x, float y, float z, float outEye[3]) {
    outEye[0] = static_cast<float>(m[0] * x + m[4] * y + m[8]  * z + m[12]);
    outEye[1] = static_cast<float>(m[1] * x + m[5] * y + m[9]  * z + m[13]);
    outEye[2] = static_cast<float>(m[2] * x + m[6] * y + m[10] * z + m[14]);
}

// Computes the cube's outward front-face normal in world space using the same
// rotation order used during drawing (Y then X calls => X then Y application)
static void ComputeCubeFrontNormalWorld(float outNormal[3]) {
    // Start from local +Z (front face)
    float x = 0.0f;
    float y = 0.0f;
    float z = 1.0f;

    // First apply X rotation
    const float pitchRad = DegreesToRadians(kCubePitchDeg);
    const float x1 = x;
    const float y1 = y * std::cos(pitchRad) - z * std::sin(pitchRad);
    const float z1 = y * std::sin(pitchRad) + z * std::cos(pitchRad);

    // Then apply Y rotation
    const float yawRad = DegreesToRadians(kCubeYawDeg);
    const float x2 = x1 * std::cos(yawRad) + z1 * std::sin(yawRad);
    const float y2 = y1;
    const float z2 = -x1 * std::sin(yawRad) + z1 * std::cos(yawRad);

    // Normalize for stable lighting offsets
    const float len = std::sqrt(x2 * x2 + y2 * y2 + z2 * z2);
    if (len > 0.0001f) {
        outNormal[0] = x2 / len;
        outNormal[1] = y2 / len;
        outNormal[2] = z2 / len;
    } else {
        outNormal[0] = 0.0f;
        outNormal[1] = 0.0f;
        outNormal[2] = 1.0f;
    }
}

// Normalizes a 3D vector in-place
static void NormalizeVec3(float v[3]) {
    const float len = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len > 0.0001f) {
        const float invLen = 1.0f / len;
        v[0] *= invLen;
        v[1] *= invLen;
        v[2] *= invLen;
    }
}

// Dot product of two 3D vectors
static float DotVec3(const float a[3], const float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// Computes a light direction that places specular highlight near the supplied
// surface point for the current camera L = -reflect(V, N)
static void ComputeSpecAlignedLightDir(const float surfacePoint[3],
                                       const float frontNormalIn[3],
                                       float outLightDir[3]) {
    float N[3] = { frontNormalIn[0], frontNormalIn[1], frontNormalIn[2] };
    NormalizeVec3(N);

    // V points from the surface point toward the camera
    float V[3] = {
        gCameraPos[0] - surfacePoint[0],
        gCameraPos[1] - surfacePoint[1],
        gCameraPos[2] - surfacePoint[2]
    };
    NormalizeVec3(V);

    // Keep the normal camera-facing so we always solve for a visible lobe
    if (DotVec3(N, V) < 0.0f) {
        N[0] = -N[0];
        N[1] = -N[1];
        N[2] = -N[2];
    }

    const float ndotv = DotVec3(N, V);
    outLightDir[0] = -V[0] + (2.0f * ndotv * N[0]);
    outLightDir[1] = -V[1] + (2.0f * ndotv * N[1]);
    outLightDir[2] = -V[2] + (2.0f * ndotv * N[2]);
    NormalizeVec3(outLightDir);
}

// Case-insensitive key-state check helper
static bool IsKeyHeld(char key) {
    const unsigned char lower = static_cast<unsigned char>(
        std::tolower(static_cast<unsigned char>(key)));
    return gKeyDown[lower];
}

// Applies movement and look deltas from currently held keys
static void UpdateCameraFromInput(float dtSeconds) {
    // Horizontal forward/right basis vectors from yaw only
    // This keeps WASD movement parallel to the ground plane
    const float yawRad = DegreesToRadians(gCameraYawDeg);
    const float forwardX = std::sin(yawRad);
    const float forwardZ = -std::cos(yawRad);
    const float rightX = std::cos(yawRad);
    const float rightZ = std::sin(yawRad);

    float moveX = 0.0f;
    float moveY = 0.0f;
    float moveZ = 0.0f;

    if (IsKeyHeld('w')) { moveX += forwardX; moveZ += forwardZ; }
    if (IsKeyHeld('s')) { moveX -= forwardX; moveZ -= forwardZ; }
    if (IsKeyHeld('a')) { moveX -= rightX;   moveZ -= rightZ;   }
    if (IsKeyHeld('d')) { moveX += rightX;   moveZ += rightZ;   }
    if (IsKeyHeld('q')) { moveY -= 1.0f; }
    if (IsKeyHeld('e')) { moveY += 1.0f; }

    // Normalize movement to keep diagonal speed equal to axial speed
    const float moveLen = std::sqrt(moveX * moveX + moveY * moveY + moveZ * moveZ);
    if (moveLen > 0.0001f) {
        const float invLen = 1.0f / moveLen;
        moveX *= invLen;
        moveY *= invLen;
        moveZ *= invLen;

        gCameraPos[0] += moveX * kCameraMoveSpeed * dtSeconds;
        gCameraPos[1] += moveY * kCameraMoveSpeed * dtSeconds;
        gCameraPos[2] += moveZ * kCameraMoveSpeed * dtSeconds;
    }

    float yawDir = 0.0f;
    float pitchDir = 0.0f;
    if (gArrowLeftDown)  yawDir   -= 1.0f;
    if (gArrowRightDown) yawDir   += 1.0f;
    if (gArrowUpDown)    pitchDir += 1.0f;
    if (gArrowDownDown)  pitchDir -= 1.0f;

    gCameraYawDeg   += yawDir   * kCameraLookSpeedDeg * dtSeconds;
    gCameraPitchDeg += pitchDir * kCameraLookSpeedDeg * dtSeconds;

    // Prevent pitch from reaching +/-90 where up vector handling becomes unstable
    gCameraPitchDeg = ClampFloat(gCameraPitchDeg, -89.0f, 89.0f);
}

// Idle callback integrate camera motion and request redraw
static void idle() {
    const int nowMs = glutGet(GLUT_ELAPSED_TIME);

    if (gPrevTimeMs == 0) {
        gPrevTimeMs = nowMs;
    }

    int deltaMs = nowMs - gPrevTimeMs;
    if (deltaMs < 0) {
        deltaMs = 0;
    }
    gPrevTimeMs = nowMs;

    float dtSeconds = static_cast<float>(deltaMs) * 0.001f;
    if (dtSeconds > kMaxDeltaTimeSeconds) {
        dtSeconds = kMaxDeltaTimeSeconds;
    }

    UpdateCameraFromInput(dtSeconds);
    glutPostRedisplay();
}

// Handles regular ASCII key press events
static void onKeyboardDown(unsigned char key, int, int) {
    const unsigned char lower = static_cast<unsigned char>(
        std::tolower(static_cast<unsigned char>(key)));

    if (lower == 27) { // ESC to quit
        std::exit(0);
    }

    gKeyDown[lower] = true;
}

// Handles regular ASCII key release events
static void onKeyboardUp(unsigned char key, int, int) {
    const unsigned char lower = static_cast<unsigned char>(
        std::tolower(static_cast<unsigned char>(key)));
    gKeyDown[lower] = false;
}

// Handles special-key press events (arrow keys)
static void onSpecialDown(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT:  gArrowLeftDown  = true; break;
        case GLUT_KEY_RIGHT: gArrowRightDown = true; break;
        case GLUT_KEY_UP:    gArrowUpDown    = true; break;
        case GLUT_KEY_DOWN:  gArrowDownDown  = true; break;
        default: break;
    }
}

// Handles special-key release events (arrow keys)
static void onSpecialUp(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT:  gArrowLeftDown  = false; break;
        case GLUT_KEY_RIGHT: gArrowRightDown = false; break;
        case GLUT_KEY_UP:    gArrowUpDown    = false; break;
        case GLUT_KEY_DOWN:  gArrowDownDown  = false; break;
        default: break;
    }
}

static bool LoadGLProcAddresses() {
#define LOAD_GL_PROC(ptr, type, nameString) \
    do { \
        ptr = reinterpret_cast<type>(glutGetProcAddress(nameString)); \
        if (!(ptr)) { \
            std::fprintf(stderr, "Missing OpenGL symbol: %s\n", nameString); \
            return false; \
        } \
    } while (0)

    LOAD_GL_PROC(pglCreateShader,       PFNGLCREATESHADERPROC,       "glCreateShader");
    LOAD_GL_PROC(pglShaderSource,       PFNGLSHADERSOURCEPROC,       "glShaderSource");
    LOAD_GL_PROC(pglCompileShader,      PFNGLCOMPILESHADERPROC,      "glCompileShader");
    LOAD_GL_PROC(pglGetShaderiv,        PFNGLGETSHADERIVPROC,        "glGetShaderiv");
    LOAD_GL_PROC(pglGetShaderInfoLog,   PFNGLGETSHADERINFOLOGPROC,   "glGetShaderInfoLog");
    LOAD_GL_PROC(pglDeleteShader,       PFNGLDELETESHADERPROC,       "glDeleteShader");
    LOAD_GL_PROC(pglCreateProgram,      PFNGLCREATEPROGRAMPROC,      "glCreateProgram");
    LOAD_GL_PROC(pglAttachShader,       PFNGLATTACHSHADERPROC,       "glAttachShader");
    LOAD_GL_PROC(pglLinkProgram,        PFNGLLINKPROGRAMPROC,        "glLinkProgram");
    LOAD_GL_PROC(pglGetProgramiv,       PFNGLGETPROGRAMIVPROC,       "glGetProgramiv");
    LOAD_GL_PROC(pglGetProgramInfoLog,  PFNGLGETPROGRAMINFOLOGPROC,  "glGetProgramInfoLog");
    LOAD_GL_PROC(pglUseProgram,         PFNGLUSEPROGRAMPROC,         "glUseProgram");
    LOAD_GL_PROC(pglGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC, "glGetUniformLocation");
    LOAD_GL_PROC(pglUniform3f,          PFNGLUNIFORM3FPROC,          "glUniform3f");
    LOAD_GL_PROC(pglUniform1f,          PFNGLUNIFORM1FPROC,          "glUniform1f");
    LOAD_GL_PROC(pglDetachShader,       PFNGLDETACHSHADERPROC,       "glDetachShader");
    LOAD_GL_PROC(pglDeleteProgram,      PFNGLDELETEPROGRAMPROC,      "glDeleteProgram");

#undef LOAD_GL_PROC
    return true;
}

static GLuint CompileShader(GLenum shaderType, const char* source) {
    GLuint shader = pglCreateShader(shaderType);
    if (!shader) {
        std::fprintf(stderr, "Failed to create shader object.\n");
        return 0;
    }

    const GLchar* src = reinterpret_cast<const GLchar*>(source);
    pglShaderSource(shader, 1, &src, nullptr);
    pglCompileShader(shader);

    GLint compiled = GL_FALSE;
    pglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLint logLen = 0;
        pglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 1) {
            std::vector<GLchar> log(static_cast<size_t>(logLen));
            pglGetShaderInfoLog(shader, logLen, nullptr, log.data());
            std::fprintf(stderr, "Shader compile error:\n%s\n", log.data());
        } else {
            std::fprintf(stderr, "Shader compile failed with no info log.\n");
        }
        pglDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = pglCreateProgram();
    if (!program) {
        std::fprintf(stderr, "Failed to create shader program.\n");
        return 0;
    }

    pglAttachShader(program, vertexShader);
    pglAttachShader(program, fragmentShader);
    pglLinkProgram(program);

    GLint linked = GL_FALSE;
    pglGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint logLen = 0;
        pglGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 1) {
            std::vector<GLchar> log(static_cast<size_t>(logLen));
            pglGetProgramInfoLog(program, logLen, nullptr, log.data());
            std::fprintf(stderr, "Program link error:\n%s\n", log.data());
        } else {
            std::fprintf(stderr, "Program link failed with no info log.\n");
        }
        pglDeleteProgram(program);
        return 0;
    }

    // Program now owns attached shader binaries we can detach shaders
    pglDetachShader(program, vertexShader);
    pglDetachShader(program, fragmentShader);
    return program;
}

static bool BuildPhongProgram() {
    // GLSL 120 keeps compatibility with legacy OpenGL contexts and immediate mode
    static const char* kVertexShaderSrc =
        "#version 120\n"
        "varying vec3 vNormalEye;\n"
        "varying vec3 vPositionEye;\n"
        "void main() {\n"
        "    vec4 posEye = gl_ModelViewMatrix * gl_Vertex;\n"
        "    vPositionEye = posEye.xyz;\n"
        "    vNormalEye = normalize(gl_NormalMatrix * gl_Normal);\n"
        "    gl_Position = gl_ProjectionMatrix * posEye;\n"
        "}\n";

    static const char* kFragmentShaderSrc =
        "#version 120\n"
        "varying vec3 vNormalEye;\n"
        "varying vec3 vPositionEye;\n"
        "\n"
        "uniform vec3 uLightPosEye;\n"
        "uniform vec3 uLightAmbient;\n"
        "uniform vec3 uLightDiffuse;\n"
        "uniform vec3 uLightSpecular;\n"
        "uniform vec3 uMatAmbient;\n"
        "uniform vec3 uMatDiffuse;\n"
        "uniform vec3 uMatSpecular;\n"
        "uniform float uMatShininess;\n"
        "uniform float uSpecularBoost;\n"
        "\n"
        "void main() {\n"
        "    vec3 N = normalize(vNormalEye);\n"
        "    vec3 L = normalize(uLightPosEye - vPositionEye);\n"
        "    vec3 V = normalize(-vPositionEye);\n"
        "\n"
        "    float NdotL = max(dot(N, L), 0.0);\n"
        "    vec3 ambient = uLightAmbient * uMatAmbient;\n"
        "    vec3 diffuse = uLightDiffuse * uMatDiffuse * NdotL;\n"
        "\n"
        "    // Phong-style specular term higher shininess -> tighter highlight\n"
        "    float spec = 0.0;\n"
        "    if (NdotL > 0.0) {\n"
        "        vec3 R = reflect(-L, N);\n"
        "        spec = pow(max(dot(R, V), 0.0), uMatShininess);\n"
        "    }\n"
        "    vec3 specular = uLightSpecular * uMatSpecular * spec * uSpecularBoost;\n"
        "\n"
        "    gl_FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
        "}\n";

    const GLuint vs = CompileShader(GL_VERTEX_SHADER,   kVertexShaderSrc);
    const GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSrc);
    if (!vs || !fs) {
        if (vs) pglDeleteShader(vs);
        if (fs) pglDeleteShader(fs);
        return false;
    }

    gPhongProgram = LinkProgram(vs, fs);

    // Shader objects are no longer needed after successful linking
    pglDeleteShader(vs);
    pglDeleteShader(fs);

    if (!gPhongProgram) {
        return false;
    }

    // Cache uniform locations once
    gULightPosEye   = pglGetUniformLocation(gPhongProgram, "uLightPosEye");
    gULightAmbient  = pglGetUniformLocation(gPhongProgram, "uLightAmbient");
    gULightDiffuse  = pglGetUniformLocation(gPhongProgram, "uLightDiffuse");
    gULightSpecular = pglGetUniformLocation(gPhongProgram, "uLightSpecular");
    gUMatAmbient    = pglGetUniformLocation(gPhongProgram, "uMatAmbient");
    gUMatDiffuse    = pglGetUniformLocation(gPhongProgram, "uMatDiffuse");
    gUMatSpecular   = pglGetUniformLocation(gPhongProgram, "uMatSpecular");
    gUMatShininess  = pglGetUniformLocation(gPhongProgram, "uMatShininess");
    gUSpecularBoost = pglGetUniformLocation(gPhongProgram, "uSpecularBoost");

    const bool missingUniform =
        (gULightPosEye   < 0) || (gULightAmbient  < 0) || (gULightDiffuse  < 0) ||
        (gULightSpecular < 0) || (gUMatAmbient    < 0) || (gUMatDiffuse    < 0) ||
        (gUMatSpecular   < 0) || (gUMatShininess  < 0) || (gUSpecularBoost < 0);

    if (missingUniform) {
        std::fprintf(stderr, "Failed to fetch one or more shader uniform locations.\n");
        pglDeleteProgram(gPhongProgram);
        gPhongProgram = 0;
        return false;
    }

    // Upload constant light/material terms once (shininess is per-cube and set per draw)
    pglUseProgram(gPhongProgram);
    pglUniform3f(gULightAmbient,  kLightAmbient[0],  kLightAmbient[1],  kLightAmbient[2]);
    pglUniform3f(gULightDiffuse,  kLightDiffuse[0],  kLightDiffuse[1],  kLightDiffuse[2]);
    pglUniform3f(gULightSpecular, kLightSpecular[0], kLightSpecular[1], kLightSpecular[2]);
    pglUniform3f(gUMatAmbient,    kMatAmbient[0],    kMatAmbient[1],    kMatAmbient[2]);
    pglUniform3f(gUMatDiffuse,    kMatDiffuse[0],    kMatDiffuse[1],    kMatDiffuse[2]);
    pglUniform3f(gUMatSpecular,   kMatSpecular[0],   kMatSpecular[1],   kMatSpecular[2]);
    pglUniform1f(gUSpecularBoost, kSpecularBoost);
    pglUseProgram(0);

    return true;
}

static void ShutdownPhongProgram() {
    // Signal the input thread to stop looping (in case it is still waiting for input)
    gInputThreadShouldExit.store(true);

    if (pglUseProgram) {
        pglUseProgram(0);
    }
    if (gPhongProgram && pglDeleteProgram) {
        pglDeleteProgram(gPhongProgram);
        gPhongProgram = 0;
    }
}

// Computes world-space X center for a column index (03)
static float CubeCenterX(int col) {
    const float centerOffset = (kCols - 1) * 0.5f;
    return (static_cast<float>(col) - centerOffset) * kColSpacing;
}

// Computes world-space Y center for a row index (0 = top, 1 = bottom)
static float CubeCenterY(int row) {
    const float centerOffset = (kRows - 1) * 0.5f;
    return (centerOffset - static_cast<float>(row)) * kRowSpacing;
}

// Returns bitmap text width in pixels for centering labels
static int BitmapStringWidth(void* font, const char* text) {
    int width = 0;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        width += glutBitmapWidth(font, *p);
    }
    return width;
}

// Draws bitmap text at 2D screen coordinates (current projection/modelview expected to be 2D-friendly)
static void DrawBitmapString2D(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        glutBitmapCharacter(font, *p);
    }
}

// Draws a unit cube centered at origin with correct per-face normals
// Immediate mode is used intentionally (legacy fixed-function request)
static void DrawUnitCube() {
    const GLfloat h = 0.5f;

    glBegin(GL_QUADS);

    // Front (+Z)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-h, -h,  h);
    glVertex3f( h, -h,  h);
    glVertex3f( h,  h,  h);
    glVertex3f(-h,  h,  h);

    // Back (-Z)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f( h, -h, -h);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h,  h, -h);
    glVertex3f( h,  h, -h);

    // Left (-X)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, -h,  h);
    glVertex3f(-h,  h,  h);
    glVertex3f(-h,  h, -h);

    // Right (+X)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( h, -h,  h);
    glVertex3f( h, -h, -h);
    glVertex3f( h,  h, -h);
    glVertex3f( h,  h,  h);

    // Top (+Y)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-h,  h,  h);
    glVertex3f( h,  h,  h);
    glVertex3f( h,  h, -h);
    glVertex3f(-h,  h, -h);

    // Bottom (-Y)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f( h, -h, -h);
    glVertex3f( h, -h,  h);
    glVertex3f(-h, -h,  h);

    glEnd();
}

// Draws all cube labels in screen space so they stay crisp and avoid depth conflicts
static void DrawLabelsOverlay(const GLdouble model[16],
                              const GLdouble proj[16],
                              const GLint viewport[4]) {
    const void* font = GLUT_BITMAP_HELVETICA_18;

    // Disable depth for flat overlay text
    glDisable(GL_DEPTH_TEST);

    // Switch to pixel-like 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(gWindowWidth),
               0.0, static_cast<double>(gWindowHeight));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Light gray/white labels
    glColor3f(0.93f, 0.93f, 0.93f);

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const float x = CubeCenterX(col);
            const float y = CubeCenterY(row) - (kCubeSize * 0.80f + 0.45f);
            const float z = 0.0f;

            GLdouble sx = 0.0, sy = 0.0, sz = 0.0;
            gluProject(static_cast<GLdouble>(x),
                       static_cast<GLdouble>(y),
                       static_cast<GLdouble>(z),
                       model, proj, viewport,
                       &sx, &sy, &sz);

            const char* label = kLabelText[row][col];
            const int textWidth = BitmapStringWidth(const_cast<void*>(font), label);

            // Center text under each cube
            const float drawX = static_cast<float>(sx) - (textWidth * 0.5f);
            const float drawY = static_cast<float>(sy) - 8.0f;

            DrawBitmapString2D(drawX, drawY, const_cast<void*>(font), label);
        }
    }

    // --- Label for the query cube (only when active) ---
    const float qs = gQueryShininess.load();
    if (qs > 0.0f) {
        // Same label Y anchor formula as grid cubes, using the query cube's world position
        const float qx = 0.0f;
        const float qy = CubeCenterY(1) - kRowSpacing - (kCubeSize * 0.80f + 0.45f);
        const float qz = 0.0f;

        GLdouble sx = 0.0, sy = 0.0, sz = 0.0;
        gluProject(static_cast<GLdouble>(qx),
                   static_cast<GLdouble>(qy),
                   static_cast<GLdouble>(qz),
                   model, proj, viewport,
                   &sx, &sy, &sz);

        // Build label string: "Query: 64" or "Query: 64.5" for non-integer values
        char queryLabel[32];
        if (qs == std::floor(qs)) {
            std::snprintf(queryLabel, sizeof(queryLabel), "Query: %d",
                          static_cast<int>(qs));
        } else {
            std::snprintf(queryLabel, sizeof(queryLabel), "Query: %.1f", qs);
        }

        // Use a slightly brighter color to visually distinguish the query label
        glColor3f(1.0f, 0.85f, 0.30f); // warm yellow, distinct from the grid's gray

        const int textWidth = BitmapStringWidth(const_cast<void*>(font), queryLabel);
        const float drawX = static_cast<float>(sx) - (textWidth * 0.5f);
        const float drawY = static_cast<float>(sy) - 8.0f;
        DrawBitmapString2D(drawX, drawY, const_cast<void*>(font), queryLabel);

        // Restore the grid label color for any future labels
        glColor3f(0.93f, 0.93f, 0.93f);
    }

    // Restore matrices and states
    glPopMatrix(); // modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}

// Initializes OpenGL render state and the Phong shader program
static void init() {
    // Flat dark gray background to match reference
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    // Core state still needed in shader path
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH); // Harmless here shader computes actual lighting

    if (!LoadGLProcAddresses()) {
        std::fprintf(stderr, "Unable to load required OpenGL shader functions.\n");
        std::exit(EXIT_FAILURE);
    }
    if (!BuildPhongProgram()) {
        std::fprintf(stderr, "Failed to build Phong shader program.\n");
        std::exit(EXIT_FAILURE);
    }

    // Initialize timer baseline for the camera update loop
    gPrevTimeMs = glutGet(GLUT_ELAPSED_TIME);
}

// Reshape callback updates viewport and perspective projection
static void reshape(int w, int h) {
    gWindowWidth  = (w > 0) ? w : 1;
    gWindowHeight = (h > 0) ? h : 1;

    glViewport(0, 0, gWindowWidth, gWindowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Slight perspective view similar to the reference image
    const double aspect = static_cast<double>(gWindowWidth) / static_cast<double>(gWindowHeight);
    gluPerspective(45.0, aspect, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

// Main display callback
static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera transform from interactive position + yaw/pitch
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const float yawRad = DegreesToRadians(gCameraYawDeg);
    const float pitchRad = DegreesToRadians(gCameraPitchDeg);

    // Direction vector built from yaw/pitch angles
    const float forwardX = std::sin(yawRad) * std::cos(pitchRad);
    const float forwardY = std::sin(pitchRad);
    const float forwardZ = -std::cos(yawRad) * std::cos(pitchRad);

    gluLookAt(
        gCameraPos[0], gCameraPos[1], gCameraPos[2],                                  // eye
        gCameraPos[0] + forwardX, gCameraPos[1] + forwardY, gCameraPos[2] + forwardZ, // center
        0.00, 1.00, 0.00                                                             // up
    );

    // Bind Phong program for cube rendering
    pglUseProgram(gPhongProgram);

    // Capture current view matrix so we can convert per-cube light positions
    // from world space to eye space once per cube
    GLdouble viewMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewMatrix);

    // Draw all 8 cubes
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const float cubeX = CubeCenterX(col);
            const float cubeY = CubeCenterY(row);
            const float cubeZ = 0.0f;

            // Use each cube's front-face center as the target point where we want
            // a visible specular lobe, then solve light direction from camera
            float frontNormal[3];
            ComputeCubeFrontNormalWorld(frontNormal);

            const float faceCenter[3] = {
                cubeX + frontNormal[0] * (kCubeSize * 0.5f),
                cubeY + frontNormal[1] * (kCubeSize * 0.5f),
                cubeZ + frontNormal[2] * (kCubeSize * 0.5f)
            };
            float lightDir[3];
            ComputeSpecAlignedLightDir(faceCenter, frontNormal, lightDir);

            const float lightWorldX = faceCenter[0] + lightDir[0] * kPerCubeLightDistance;
            const float lightWorldY = faceCenter[1] + lightDir[1] * kPerCubeLightDistance;
            const float lightWorldZ = faceCenter[2] + lightDir[2] * kPerCubeLightDistance;
            float lightEye[3];
            TransformPointByMatrix(viewMatrix, lightWorldX, lightWorldY, lightWorldZ, lightEye);
            pglUniform3f(gULightPosEye, lightEye[0], lightEye[1], lightEye[2]);

            // In Phong-style shading, larger shininess values produce smaller,
            // more concentrated specular highlights
            const float shininess = EffectiveShininessFromLabel(kShininessLabels[row][col]);
            pglUniform1f(gUMatShininess, shininess);

            glPushMatrix();

            // Place cube in the 2x4 world-space grid
            glTranslatef(cubeX, cubeY, cubeZ);

            // Rotate so front and right faces are both visible, matching the reference style
            glRotatef(kCubeYawDeg,   0.0f, 1.0f, 0.0f);
            glRotatef(kCubePitchDeg, 1.0f, 0.0f, 0.0f);

            // Uniform scaling from unit cube to desired size
            glScalef(kCubeSize, kCubeSize, kCubeSize);
            DrawUnitCube();

            glPopMatrix();
        }
    }

    // --- Query cube (9th cube, rendered only when a value has been submitted) ---
    const float queryShininess = gQueryShininess.load();
    if (queryShininess > 0.0f) {
        const float queryCubeX = 0.0f;
        const float queryCubeY = CubeCenterY(1) - kRowSpacing; // centered below grid
        const float queryCubeZ = 0.0f;

        // Reuse the same per-face-center specular-aligned light as the grid cubes
        float frontNormal[3];
        ComputeCubeFrontNormalWorld(frontNormal);

        const float faceCenter[3] = {
            queryCubeX + frontNormal[0] * (kCubeSize * 0.5f),
            queryCubeY + frontNormal[1] * (kCubeSize * 0.5f),
            queryCubeZ + frontNormal[2] * (kCubeSize * 0.5f)
        };
        float lightDir[3];
        ComputeSpecAlignedLightDir(faceCenter, frontNormal, lightDir);

        const float lightWorldX = faceCenter[0] + lightDir[0] * kPerCubeLightDistance;
        const float lightWorldY = faceCenter[1] + lightDir[1] * kPerCubeLightDistance;
        const float lightWorldZ = faceCenter[2] + lightDir[2] * kPerCubeLightDistance;
        float lightEye[3];
        TransformPointByMatrix(viewMatrix, lightWorldX, lightWorldY, lightWorldZ, lightEye);
        pglUniform3f(gULightPosEye, lightEye[0], lightEye[1], lightEye[2]);
        pglUniform1f(gUMatShininess, queryShininess);

        glPushMatrix();
        glTranslatef(queryCubeX, queryCubeY, queryCubeZ);
        glRotatef(kCubeYawDeg,   0.0f, 1.0f, 0.0f);
        glRotatef(kCubePitchDeg, 1.0f, 0.0f, 0.0f);
        glScalef(kCubeSize, kCubeSize, kCubeSize);
        DrawUnitCube();
        glPopMatrix();
    }

    // Return to fixed pipeline before drawing GLUT bitmap text
    pglUseProgram(0);

    // Capture current 3D transforms to project label anchor points
    GLdouble model[16];
    GLdouble proj[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Draw numeric shininess labels under each cube
    DrawLabelsOverlay(model, proj, viewport);

    glutSwapBuffers();
}

// Runs on a background thread. Reads shininess values from stdin and posts
// them to gQueryShininess for the render thread to consume.
static void InputThreadFunc() {
    char buf[64];
    while (!gInputThreadShouldExit.load()) {
        std::fprintf(stdout, "Enter shininess value (1-1000, 0 to quit): ");
        std::fflush(stdout);

        if (!std::fgets(buf, sizeof(buf), stdin)) {
            // EOF or read error — stop prompting
            break;
        }

        // Parse the entered number
        float value = 0.0f;
        try {
            value = std::stof(std::string(buf));
        } catch (...) {
            std::fprintf(stdout, "  Invalid input, please enter a number.\n");
            continue;
        }

        if (value <= 0.0f) {
            // 0 or negative ends the prompt loop; does NOT quit the program
            std::fprintf(stdout, "  Query input ended. Press ESC in the window to quit.\n");
            break;
        }

        // Clamp to the allowed range before storing
        if (value > 1000.0f) value = 1000.0f;

        gQueryShininess.store(value);
        std::fprintf(stdout, "  Query cube updated: shininess = %.1f\n", value);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(gWindowWidth, gWindowHeight);
    glutCreateWindow("Specular Lighting, Objects, Illumination and Shaders");

    init();
    std::atexit(ShutdownPhongProgram);

    // Start the background console input thread AFTER the OpenGL context is
    // initialized (init() runs first) so that gQueryShininess is visible.
    gInputThread = std::thread(InputThreadFunc);
    // Detach so glutMainLoop() can block freely; the thread ends on its own
    // when the user types 0 or closes stdin.
    gInputThread.detach();

    // Register keyboard and idle callbacks for the camera control scheme
    glutKeyboardFunc(onKeyboardDown);
    glutKeyboardUpFunc(onKeyboardUp);
    glutSpecialFunc(onSpecialDown);
    glutSpecialUpFunc(onSpecialUp);
    glutIdleFunc(idle);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}

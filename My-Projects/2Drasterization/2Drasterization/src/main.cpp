#include "framework/Framebuffer.h"
#include "core/Color.h"
#include "core/Vec3.h"
#include "core/Vec4.h"
#include "core/Matrix4.h"
#include "scene/Camera.h"
#include "pipeline/GridRenderer.h"
#include "pipeline/GeometryUtils.h"
#include "pipeline/Renderer.h" 
#include "gpu/CudaRenderer.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <chrono>

using namespace GeometryUtils;

#pragma region vertex & fragment shaders

// Vertex shader source for a full-screen quad.
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment shader source that simply samples a texture.
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D screenTexture;
void main()
{
    FragColor = texture(screenTexture, TexCoord);
}
)";

#pragma endregion

#pragma region shader compilation and linking
// Utility function to compile and link the shader program.
GLuint compileShaderProgram() {
    GLint success;
    GLchar infoLog[512];

    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    // Shader Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    // Cleanup shaders after linking.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
#pragma endregion  
// Viewing frustum
static float near = 1.0f;  // near clipping plane distance
static float far = 100.f;   // far clipping plane

// Camera vars
static float cameraYaw = -90.0f;   // degrees, around Y
static float cameraPitch = 0.f;    // degrees, up/down
static const Vec3 cameraPos(0.f, -10.f, 30.f);

static float moveSpeed = 30.f;
static float sense = 0.05f;
static float zoom = 40.f;

// Mouse vars
static bool   firstMouse = true;
static double lastX = 0.0, lastY = 0.0;

// Grid bounds
int gridMaxX = 30, gridMinX = -30;
int gridMaxZ = 30, gridMinZ = -30;

float deg2rad(float d) { return d * 3.14159265f / 180.0f; }


int main()
{
#pragma region window & GLFW texture
    // Initialize GLFW.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    // Set up OpenGL version (3.3 Core Profile).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window.
    GLFWwindow* window = glfwCreateWindow(1200, 1000, "My Renderer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Compile the shader program.
    GLuint shaderProgram = compileShaderProgram();

    // Set up a full-screen quad.
    float quadVertices[] = {
        // positions   // texture coordinates
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
         1.0f,  1.0f,   1.0f, 1.0f,  // top-right
         1.0f, -1.0f,   1.0f, 0.0f   // bottom-right
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Set attribute pointers.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Create an OpenGL texture to display the framebuffer.
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Texture parameters for pixel-perfect display.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int fbWidth = 1200, fbHeight = 1000;
    // Allocate empty texture storage.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
#pragma endregion
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Create a camera
    Camera cam(cameraPos, { 0, 1, 0 }, cameraYaw, cameraPitch, moveSpeed, sense, zoom);

    // Create software framebuffer
    Framebuffer fb(fbWidth, fbHeight);

    CudaRenderer cudaR(fbWidth, fbHeight, /*maxPrims=*/20000);

    // Create renderer
    Renderer renderer(fbWidth, fbHeight);

    // Prepare a scene with several triangles
    std::vector<Mesh> sceneMeshes;
    Mesh m1({ Triangle3D(Vec3(12.0f, -10.f, 6.0f),
        Vec3(11.0f, -10.f, 9.0f),
        Vec3(10.0f, -11.0f, 7.0f),
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255)) });
    Mesh m2({ Triangle3D(Vec3(6.0f, -7.0f, -4.0f),
        Vec3(11.0f, -10.0f, -1.0f),
        Vec3(10.0f, -11.0f, -3.0f),
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255)) });
    Mesh m3;
    // Polygon test
    Vec4Poly fanTest = { {0.0f, 0.0f, 0.0f, 1.0f}, {6.0f, 0.0f, 0.0f, 1.0f},
         {8.0f, -4.0f, 0.0f, 1.0f}, { 3.0f, -7.0f, 0.0f, 1.0f } , {-2.0f, -4.0f, 0.0f, 1.0f} };

    std::vector<Tri4> testpoly = triangulateFan(fanTest);
    for (Tri4 tri : testpoly)
    {
        m3.triangles.emplace_back(
            tri[0].toVec3(), tri[1].toVec3(), tri[2].toVec3(),
            Color(155, 0, 255),
            Color(155, 0, 255),
            Color(155, 0, 255)
        );
    }
    //*
    sceneMeshes.emplace_back(m1); 
    sceneMeshes.emplace_back(m2); 
    sceneMeshes.emplace_back(m3); 

    // Create grid
    static GridRenderer grid(gridMinX, gridMaxX, gridMinZ, gridMaxZ, 2.0f);
    // sceneMeshes.emplace_back(Mesh(grid.generateGridQuads(0.1f)));

    // Time information
    float lastTime = glfwGetTime();

    // Main loop.
    while (!glfwWindowShouldClose(window))
    {
        // Process input/events here
        float now = glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        glfwPollEvents();

        // Close window on escape
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        // KB movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam.processKeyboard(MoveDir::Forward, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam.processKeyboard(MoveDir::Backward, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam.processKeyboard(MoveDir::Left, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam.processKeyboard(MoveDir::Right, dt);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cam.processKeyboard(MoveDir::Up, dt);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            cam.processKeyboard(MoveDir::Down, dt);
        //*

        // Mouse movement
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        double dx = xpos - lastX;
        double dy = lastY - ypos;

        cam.processMouseMovement(dx, dy, true);

        lastX = xpos;
        lastY = ypos;
        //* 
        
        // Aspect ratio
        float aspect = float(fb.getWidth()) / float(fb.getHeight()); 

        auto const& prims = renderer.preparePrimitives(cam, deg2rad(cam.getZoom()), 
            aspect, near, far, sceneMeshes);

        cudaR.render(prims, fb);
        
        // Full entity transform and clipping
        // renderer.render(cam, deg2rad(cam.getZoom()), aspect, near, far, sceneMeshes, fb);

        // Draw grid
        // grid.draw(cam, fb, near);

    #pragma region update/render to window
        // Update the OpenGL texture with the framebuffer's color buffer.
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fbWidth, fbHeight, GL_RGB, GL_UNSIGNED_BYTE, fb.getColorBuffer().data());

        // Render the full-screen quad.
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Swap buffers and poll for events.
        glfwSwapBuffers(window);
#pragma endregion
    }

    // Cleanup resources.
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

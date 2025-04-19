#include "Triangle.h"
#include "Framebuffer.h"
#include "Color.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Matrix4.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>

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
static Matrix4 gViewMatrix = Matrix4::identity();

// --- at the top, before main() ---
static float cameraYaw = -90.0f;   // degrees, around Y
static float cameraPitch = 20.0f;   // degrees, up/down
static float cameraDist = 5.0f;   // radius from target
static const Vec3 cameraTarget(0.0f, 0.0f, 0.0f);

// helper to convert degrees→radians
inline float deg2rad(float d) { return d * 3.14159265f / 180.0f; }

// call this each frame before computing V
void updateCameraFromInput(GLFWwindow* window, float deltaTime) {
    float speedAng = 45.0f;   // deg/sec
    float speedZoom = 2.0f;  // units/sec

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraYaw -= speedAng * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraYaw += speedAng * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPitch += speedAng * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPitch -= speedAng * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) cameraDist -= speedZoom * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) cameraDist += speedZoom * deltaTime;

    // clamp
    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;
    if (cameraDist < 1.0f) cameraDist = 1.0f;

    // compute Cartesian camera position from spherical coords
    float yawRad = deg2rad(cameraYaw);
    float pitchRad = deg2rad(cameraPitch);

    Vec3 camPos;
    camPos.x_ = cameraTarget.x_ + cameraDist * std::cos(pitchRad) * std::cos(yawRad);
    camPos.y_ = cameraTarget.y_ + cameraDist * std::sin(pitchRad);
    camPos.z_ = cameraTarget.z_ + cameraDist * std::cos(pitchRad) * std::sin(yawRad);

    // update your view matrix
    gViewMatrix = Matrix4::lookAt(camPos, cameraTarget, Vec3(0, 1, 0));
}

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
    GLFWwindow* window = glfwCreateWindow(500, 500, "My Renderer", nullptr, nullptr);
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

    int fbWidth = 500, fbHeight = 500;
    // Allocate empty texture storage.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
#pragma endregion

    // Create your software framebuffer.
    Framebuffer fb(fbWidth, fbHeight);

    // Prepare a scene with several triangles.
    std::vector<Triangle> worldScene;
    worldScene.emplace_back(
        Vec3(-1.0f, -1.0f, -4.0f),
        Vec3(1.0f, -1.0f, -1.0f),
        Vec3(0.0f, 1.0f, -1.0f),
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255)
    );
    worldScene.emplace_back(
        Vec3(-4.0f, -3.0f, -4.0f),
        Vec3(1.0f, -1.0f, -1.0f),
        Vec3(0.0f, 1.0f, -3.0f),
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255)
    );

    auto toScreen = [&](const Vec4& ndc)
        {
            float x = (ndc.x() * 0.5f + 0.5f) * fb.getWidth();
            float y = (1.0f - (ndc.y() * 0.5f + 0.5f)) * fb.getHeight();
            float z = ndc.z();
            return Vec3(x, y, z);
        };

    float lastTime = glfwGetTime();

    // Main loop.
    while (!glfwWindowShouldClose(window))
    {
        // Process input/events here.
        float now = glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        glfwPollEvents();
        updateCameraFromInput(window, dt);

        // MVP
        float fovY = 1.0472f;
        float aspect = float(fb.getWidth()) / float(fb.getHeight());
        float near = 0.1f;
        float far = 100.0f;

        Matrix4 P = Matrix4::perspective(fovY, aspect, near, far);

        Matrix4 M = Matrix4::identity();

        Matrix4 V = gViewMatrix;

        Matrix4 MVP = P * gViewMatrix * M;

        // Screen scene
        std::vector<Triangle> screenScene;
        screenScene.reserve(worldScene.size());
        
        for (const auto& wtri : worldScene)
        {
            // Triangle transform;
            Vec4 h0(wtri.v0.x_, wtri.v0.y_, wtri.v0.z_, 1);
            Vec4 h1(wtri.v1.x_, wtri.v1.y_, wtri.v1.z_, 1);
            Vec4 h2(wtri.v2.x_, wtri.v2.y_, wtri.v2.z_, 1);

            Vec4 c0 = MVP * h0;
            Vec4 c1 = MVP * h1;
            Vec4 c2 = MVP * h2;

            Vec4 n0 = c0.perspectiveDivide();
            Vec4 n1 = c1.perspectiveDivide();
            Vec4 n2 = c2.perspectiveDivide();

            //std::cout << "n0 = (" << n0.x() << "," << n0.y() << "," << n0.z() << ")\n";
            //std::cout << "n1 = (" << n1.x() << "," << n1.y() << "," << n1.z() << ")\n";
            //std::cout << "n2 = (" << n2.x() << "," << n2.y() << "," << n2.z() << ")\n";

            Vec3 s0 = toScreen(n0);
            Vec3 s1 = toScreen(n1);
            Vec3 s2 = toScreen(n2);

            //std::cout << "s0 = (" << s0.x_ << "," << s0.y_ << "," << s0.z_ << ")\n";
            //std::cout << "s1 = (" << s1.x_ << "," << s1.y_ << "," << s1.z_ << ")\n";
            //std::cout << "s2 = (" << s2.x_ << "," << s2.y_ << "," << s2.z_ << ")\n";

            screenScene.emplace_back(
                Vec3(s0.x_, s0.y_, s0.z_),
                Vec3(s1.x_, s1.y_, s1.z_),
                Vec3(s2.x_, s2.y_, s2.z_),
                wtri.color0,
                wtri.color1,
                wtri.color2
            );
        }

        // Clear software framebuffer.
        fb.clearColor(Color(30, 30, 30));
        fb.clearDepth(std::numeric_limits<float>::max());

        // Rasterize each triangle onto the framebuffer.
        for (const Triangle& tri : screenScene)
        {
            // Initialize and compute the bounding box for the given triangle.
            Triangle::Boundingbox bbox = tri.getBoundingBox(fb.getWidth(), fb.getHeight());

            // Loop over pixels in the bounding box.
            for (int y = bbox.minY; y <= bbox.maxY; y++)
            {
                for (int x = bbox.minX; x <= bbox.maxX; x++)
                {
                    // Compute the center of the pixel.
                    Vec3 p(x + 0.5f, y + 0.5f, 0);
                    if (tri.contains(p))
                    {
                        Triangle::Barycentrics bary = tri.computeBarycentrics(p);
                        // Interpolate depth.
                        float interpDepth = tri.interpolateDepth(bary);
                        int index = y * fb.getWidth() + x;
                        // Depth test.
                        if (interpDepth < fb.getDepthBuffer()[index]) {
                            Color interpColor = tri.interpolateColor(bary);
                            fb.setPixel(x, y, interpColor, interpDepth);
                        }
                    }
                }
            }
        }

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
        glfwPollEvents();
    }
#pragma endregion

    // Cleanup resources.
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

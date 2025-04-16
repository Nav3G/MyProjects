#include "Triangle.h"
#include "Framebuffer.h"
#include "Color.h"
#include "Vec3.h"

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
    Triangle t1({ 100, 50, 0 }, { 400, 150, 0 }, { 250, 400, 0 },
        Color(255, 0, 0), Color(0, 100, 0), Color(0, 0, 255));
    Triangle t2({ 100, 200, 10 }, { 300, 180, 10 }, { 300, 200, 10 },
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 100));
    Triangle t3({ 150, 50, 20 }, { 280, 150, 20 }, { 150, 300, 20 },
        Color(100, 0, 0), Color(0, 255, 0), Color(0, 0, 255));
    std::vector<Triangle> scene = { t1, t2, t3 };

    // Main loop.
    while (!glfwWindowShouldClose(window))
    {
        // (Optional) Process input/events here.

        // Clear your software framebuffer.
        fb.clearColor(Color(30, 30, 30));
        fb.clearDepth(std::numeric_limits<float>::max());

        // Rasterize each triangle onto the framebuffer.
        for (const Triangle& tri : scene)
        {
            // Compute the bounding box for the triangle.
            int minX = std::min({ tri.v0.x, tri.v1.x, tri.v2.x });
            int maxX = std::max({ tri.v0.x, tri.v1.x, tri.v2.x });
            int minY = std::min({ tri.v0.y, tri.v1.y, tri.v2.y });
            int maxY = std::max({ tri.v0.y, tri.v1.y, tri.v2.y });
            minX = std::max(minX, 0);
            maxX = std::min(maxX, fb.getWidth() - 1);
            minY = std::max(minY, 0);
            maxY = std::min(maxY, fb.getHeight() - 1);

            // Loop over pixels in the bounding box.
            for (int y = minY; y <= maxY; y++)
            {
                for (int x = minX; x <= maxX; x++)
                {
                    // Compute the center of the pixel.
                    Vec3 p(x + 0.5f, y + 0.5f, 0);
                    if (tri.contains(p))
                    {
                        Triangle::Barycentrics bary = tri.computeBarycentrics(p);
                        // Interpolate depth.
                        float interpDepth = bary.alpha * tri.v0.z +
                            bary.beta * tri.v1.z + bary.gamma * tri.v2.z;
                        int index = y * fb.getWidth() + x;
                        // Depth test.
                        if (interpDepth < fb.getDepthBuffer()[index]) {
                            fb.setDepthBuffer(interpDepth, y, x);
                            Color interpColor = tri.interpolateColor(bary);
                            fb.setPixel(x, y, interpColor, interpDepth);
                        }
                    }
                }
            }
        }

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

    // Cleanup resources.
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

#include "Triangle.h"
#include "Framebuffer.h"
#include "Color.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Matrix4.h"
#include "Camera.h"
#include "Grid.h"
#include "GeometryUtils.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>

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

    // Create software framebuffer.
    Framebuffer fb(fbWidth, fbHeight);

    // Prepare a scene with several triangles.
    std::vector<Triangle> worldScene;
    worldScene.emplace_back(
        Vec3(12.0f, -10.f, 6.0f),
        Vec3(11.0f, -10.f, 9.0f),
        Vec3(10.0f, -11.0f, 7.0f),
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255));
    worldScene.emplace_back(
        Vec3(6.0f, -7.0f, -4.0f),
        Vec3(11.0f, -10.0f, -1.0f),
        Vec3(10.0f, -11.0f, -3.0f),
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255));

    // Polygon test
    Vec4Poly fanTest = { {0.0f, 0.0f, 0.0f, 1.0f}, {6.0f, 0.0f, 0.0f, 1.0f},
         {8.0f, -4.0f, 0.0f, 1.0f}, { 3.0f, -7.0f, 0.0f, 1.0f } , {-2.0f, -4.0f, 0.0f, 1.0f} };

    std::vector<Tri4> testpoly = triangulateFan(fanTest);
    for (Tri4 tri : testpoly)
    {
        worldScene.emplace_back(
            tri[0].toVec3(), tri[1].toVec3(), tri[2].toVec3(),
            Color(155, 0, 255),
            Color(155, 0, 255),
            Color(155, 0, 255)
        );
    }
    //*

    // Create grid
    static Grid grid(gridMinX, gridMaxX, gridMinZ, gridMaxZ, /*spacing=*/1.0f);

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

        // PMV matrix
        float aspect = float(fb.getWidth()) / float(fb.getHeight());

        Matrix4 V = cam.getViewMatrix();
        Matrix4 P = cam.getProjMatrix(deg2rad(cam.getZoom()), aspect, near, far);
        Matrix4 M = Matrix4::identity();
        //*
        
        // Screen scene
        std::vector<Triangle> screenScene;
        screenScene.reserve(worldScene.size());
        screenScene.clear();

        // Full entity transform and clipping
        for (const auto& wtri : worldScene)
        {
            // Triangle transform
            // 1) World space --> homogenous coords, i.e. with w element = 1
            Vec4 h0(wtri.v0.x_, wtri.v0.y_, wtri.v0.z_, 1.0f);
            Vec4 h1(wtri.v1.x_, wtri.v1.y_, wtri.v1.z_, 1.0f);
            Vec4 h2(wtri.v2.x_, wtri.v2.y_, wtri.v2.z_, 1.0f);

            // 2) VM transform: scale * camera space transform
            // homogenous --> camera space
            Vec4 cam0 = V * M * h0;
            Vec4 cam1 = V * M * h1;
            Vec4 cam2 = V * M * h2;

            // 3) Back-face culling
            // Compute triangle normal in camera space
            if (cam0.z() > -near && cam1.z() > -near && cam2.z() > -near) continue;
            Vec3 normal = (cam1 - cam0).cross(cam2 - cam0);
            if (normal.z_ <= 0) continue; // skip

            // 4) Perspective transform
            // cam space --> clip space
            Vec4 c0 = P * cam0;
            Vec4 c1 = P * cam1;
            Vec4 c2 = P * cam2;

            // 4.1) Clip space check for object being fully outside frustum
            bool isOutside = false;
            bool needsClip = false;

            for (auto f : { planeLeft, planeRight, planeBottom, planeTop, planeNear, planeFar }) 
            {
                float d0 = f(c0), d1 = f(c1), d2 = f(c2);
                // 1) Entirely outside?
                if (d0 < 0 && d1 < 0 && d2 < 0) { isOutside = true; break; }
                // 2) Partially inside (i.e. some inside, some outside)?
                if (d0 < 0 || d1 < 0 || d2 < 0) { needsClip = true; }
            }

            if (isOutside) continue;  // skip the triangle altogether

            // 4.2) Perspective-correct interpolation, storing ws before any perspective divide
            float clipW[3] = { c0.w(), c1.w(), c2.w() };

            if (!needsClip) 
            {
                // case 1: fully inside —> go straight to NDC -> screen and emit one Triangle
                
                // 5i) Perspective divide to map each point to NDC on the[-1, 1] ^ 3 cube
                // Basically we divide by the w comp in clip space, which has taken on the
                // value -P_cam,z.
                // clip space --> NDC
                Vec4 n0 = c0.perspectiveDivide();
                Vec4 n1 = c1.perspectiveDivide();
                Vec4 n2 = c2.perspectiveDivide();

                // 6i) NDC --> screen space [0, w] x [0, h] for the framebuffer
                Vec3 s0 = fb.toScreen(n0);
                Vec3 s1 = fb.toScreen(n1);
                Vec3 s2 = fb.toScreen(n2);

                // 7i) Fill the screen space array with the transformed triangle
                Triangle tri(s0, s1, s2, wtri.color0, wtri.color1, wtri.color2);
                tri.preparePerspective(clipW);
                screenScene.emplace_back(tri);
            }
            else
            {
                // case 2: partial -> existing clipPolygon() + triangulateFan()
                
                // 6ii) Frustum clipping
                // Make a VectexPoly containing the clip space coords and color data
                VertexPoly poly = {
                { c0, wtri.color0 },
                { c1, wtri.color1 },
                { c2, wtri.color2 } };

                // Perform the frustum check to determine if the polygon intersects any fustum plane
                // and clip it if it does
                for (auto f : { planeLeft, planeRight, planeBottom, planeTop, planeNear, planeFar }) {
                    poly = clipPolygon(poly, f);
                    if (poly.empty()) break;
                }
                if (poly.empty()) continue;  // fully culled

                // Fan out the newly clipped polygon
                auto fans = triangulateFan(poly);

                for (auto& triV : fans)
                {
                    // 5ii) Perspective divide to map each point to NDC on the[-1, 1] ^ 3 cube
                    // Basically we divide by the w comp in clip space, which has taken on the
                    // value -P_cam,z.
                    // clip space --> NDC
                    Vec4 ndc0 = triV[0].clipPos.perspectiveDivide();
                    Vec4 ndc1 = triV[1].clipPos.perspectiveDivide();
                    Vec4 ndc2 = triV[2].clipPos.perspectiveDivide();

                    // 6ii) NDC --> screen space [0, w] x [0, h] for the framebuffer
                    Vec3 s0 = fb.toScreen(ndc0);
                    Vec3 s1 = fb.toScreen(ndc1);
                    Vec3 s2 = fb.toScreen(ndc2);

                    // Culled vertex color data
                    Color col1 = triV[1].color;
                    Color col0 = triV[0].color;
                    Color col2 = triV[2].color;

                    // 8ii) Fill the screen space array with the transformed triangle
                    Triangle tri(s0, s1, s2, col0, col1, col2);
                    tri.preparePerspective(clipW);
                    screenScene.emplace_back(tri);
                }
                //*
            }
        }

        // Clear software framebuffer.
        fb.clearColor(Color(150, 150, 150));
        fb.clearDepth(std::numeric_limits<float>::max());

        // Draw grid
        grid.draw(cam, fb, near);

        // Rasterize each triangle onto the framebuffer
        for (const Triangle& tri : screenScene)
        {
            // Initialize and compute the bounding box for the given triangle
            Triangle::Boundingbox bbox = tri.getBoundingBox(fb.getWidth(), fb.getHeight());

            // Loop over pixels in the bounding box
            for (int y = bbox.minY; y <= bbox.maxY; y++)
            {
                for (int x = bbox.minX; x <= bbox.maxX; x++)
                {
                    // Get the center of a pixel
                    Vec3 p(x + 0.5f, y + 0.5f, 0);
                    if (tri.contains(p))
                    {
                        Triangle::Barycentrics bary = tri.computeBarycentrics(p);
                        // Interpolate depth
                        float interpDepth = tri.interpolateDepth(bary);
                        int index = y * fb.getWidth() + x;
                        // Depth test
                        if (interpDepth < fb.getDepthBuffer()[index]) {
                            Color interpColor = tri.interpolateColorPC(bary);
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

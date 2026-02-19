//
//  main.cpp
//  Hello OpenGL
//
//  Created by Ridwan AlSamman on 2/12/26.
//
// 1. Foundation types first
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Variables to control properties via keyboard buttons
float xOffset = 0.0f;           // Controls Movement
float alphaValue = 0.6f;        // Controls Transparency

// Vertex Shader (Includes uOffset for movement)
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"uniform float uOffset;\n" // Uniform for X-axis movement
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x + uOffset, aPos.y, aPos.z, 1.0);\n"
"   ourColor = aColor;\n"
"}\n";

// Fragment Shader (Includes uAlpha for transparency)
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"uniform float uAlpha;\n" // Uniform for transparency
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, uAlpha);\n"
"}\n";

// Function to process 4 specific buttons
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Button 1: 'W' Key -> Change to Wireframe Mode
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Button 2: 'F' Key -> Change to Fill Mode
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Button 3: 'RIGHT' Arrow -> Movement Property
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        xOffset += 0.005f;
    
    // Button 4: 'UP' Arrow -> Modify Transparency Property
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        alphaValue += 0.005f;
        if (alphaValue > 1.0f) alphaValue = 1.0f;
    }
    
    // Extra controls to reverse the effects
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        xOffset -= 0.005f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        alphaValue -= 0.005f;
        if (alphaValue < 0.0f) alphaValue = 0.0f;
    }
}

int main()
{
    // 1. Initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // REQUIRED FOR MAC: Forward compatibility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab: Shapes, Depth, & Transparency", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    // 2. Enable Advanced Features (Depth & Blend)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3. Build Shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 4. Data: Two Triangles (Format: x, y, z, r, g, b)
    float vertices[] = {
        // Triangle 1 (Red - Close Z=0.0)
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,

        // Triangle 2 (Blue - Far Z=0.5)
        -0.2f, -0.2f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.8f, -0.2f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.3f,  0.8f, 0.5f,  0.0f, 0.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position Attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color Attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 5. Render Loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Clear Color and Depth Buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // Get uniform locations
        int alphaLoc = glGetUniformLocation(shaderProgram, "uAlpha");
        int offsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
        glUniform1f(offsetLoc, xOffset); // Apply movement

        // === Draw Opaque Objects First ===
        glUniform1f(alphaLoc, 1.0f); // Opaque
        glDrawArrays(GL_TRIANGLES, 3, 3); // Blue Triangle

        // === Draw Transparent Objects Last ===
        glUniform1f(alphaLoc, alphaValue); // Semi-transparent
        glDrawArrays(GL_TRIANGLES, 0, 3); // Red Triangle

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

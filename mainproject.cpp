#include <iostream>
#include <cmath>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// إعدادات النافذة
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// بيانات الدائرة (الرادار)
const int CIRCLE_POINTS = 100;
float radarVertices[CIRCLE_POINTS * 3];

// إعداد المتغيرات للتحكم
float rotation = 0.0f;       // زاوية دوران الشعاع
float xOffset = 0.0f;        // إزاحة X
float alpha = 1.0f;          // الشفافية
bool wireframe = false;      // وضع Wireframe

// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform float uRotation;
uniform float uXOffset;
void main()
{
    // دوران حول المركز
    float s = sin(uRotation);
    float c = cos(uRotation);
    float x = aPos.x * c - aPos.y * s;
    float y = aPos.x * s + aPos.y * c;
    gl_Position = vec4(x + uXOffset, y, aPos.z, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main()
{
    FragColor = uColor;
}
)";

// دالة تغيير حجم النافذة
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// دالة لمعالجة المدخلات
void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // تبديل Wireframe/Fill
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        wireframe = false;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        wireframe = true;

    // التحكم بالشفافية
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        alpha += 0.01f;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        alpha -= 0.01f;
    if(alpha > 1.0f) alpha = 1.0f;
    if(alpha < 0.0f) alpha = 0.0f;

    // تحريك الرادار أفقياً
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        xOffset += 0.01f;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        xOffset -= 0.01f;
}

// إنشاء نقاط الدائرة
void generateRadarVertices()
{
    for(int i = 0; i < CIRCLE_POINTS; i++)
    {
        float angle = 2.0f * M_PI * i / CIRCLE_POINTS;
        radarVertices[i * 3 + 0] = cos(angle) * 0.5f; // x
        radarVertices[i * 3 + 1] = sin(angle) * 0.5f; // y
        radarVertices[i * 3 + 2] = 0.0f;             // z
    }
}

int main()
{
    // --- تهيئة GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Radar Simulation", NULL, NULL);
    if(!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- تهيئة GLEW ---
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- إنشاء وتجميع الشيدر ---
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

    // --- إعداد بيانات الرادار ---
    generateRadarVertices();

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(radarVertices), radarVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- حلقة الرسم ---
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        // تحديث زاوية الدوران للشعاع
        rotation += 0.01f;
        if(rotation > 2.0f * M_PI) rotation -= 2.0f * M_PI;

        // تحديد Wireframe/Fill
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // إرسال Uniforms
        int uRotLoc = glGetUniformLocation(shaderProgram, "uRotation");
        int uXLoc = glGetUniformLocation(shaderProgram, "uXOffset");
        int uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
        glUniform1f(uRotLoc, rotation);
        glUniform1f(uXLoc, xOffset);
        glUniform4f(uColorLoc, 0.0f, 1.0f, 0.0f, alpha);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_POINTS); // دائرة الرادار

        // رسم الشعاع
        float rayVertices[] = {0.0f, 0.0f, 0.0f, cos(rotation) * 0.5f, sin(rotation) * 0.5f, 0.0f};
        unsigned int rayVBO, rayVAO;
        glGenVertexArrays(1, &rayVAO);
        glGenBuffers(1, &rayVBO);
        glBindVertexArray(rayVAO);
        glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), rayVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_LINES, 0, 2);

        glDeleteVertexArrays(1, &rayVAO);
        glDeleteBuffers(1, &rayVBO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

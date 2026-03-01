#include <iostream>
#include <cmath>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int CIRCLE_POINTS = 100;

float rotationAngle = 0.0f;

// Wave
float waveX = 0.0f;
float waveY = 0.0f;
float waveRadius = 0.0f;
float waveAlpha = 0.0f;
float waveAngle = 0.0f;
bool showWave = false;
bool detected = false;

// ================= SHADERS =================

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos, 1.0);
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

// ===========================================

void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}

void generateCircle(std::vector<float>& vertices, float radius)
{
    vertices.clear();
    for(int i = 0; i < CIRCLE_POINTS; i++)
    {
        float angle = 2.0f * M_PI * i / CIRCLE_POINTS;
        vertices.push_back(cos(angle) * radius);
        vertices.push_back(sin(angle) * radius);
        vertices.push_back(0.0f);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float x = (2.0f * xpos) / SCR_WIDTH - 1.0f;
        float y = 1.0f - (2.0f * ypos) / SCR_HEIGHT;

        if(sqrt(x*x + y*y) <= 0.5f)
        {
            waveX = x;
            waveY = y;
            waveRadius = 0.0f;
            waveAlpha = 0.2f;
            waveAngle = atan2(y, x);
            showWave = true;
            detected = false;
        }
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Advanced Radar", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint colorLoc = glGetUniformLocation(shader, "uColor");

    // ===== Main Circle =====
    std::vector<float> mainCircle;
    generateCircle(mainCircle, 0.5f);

    GLuint radarVAO, radarVBO;
    glGenVertexArrays(1, &radarVAO);
    glGenBuffers(1, &radarVBO);

    glBindVertexArray(radarVAO);
    glBindBuffer(GL_ARRAY_BUFFER, radarVBO);
    glBufferData(GL_ARRAY_BUFFER, mainCircle.size()*sizeof(float), mainCircle.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    // ===== Inner Circles =====
    std::vector<float> inner1, inner2, inner3;
    generateCircle(inner1, 0.15f);
    generateCircle(inner2, 0.30f);
    generateCircle(inner3, 0.40f);

    GLuint innerVAO[3], innerVBO[3];
    glGenVertexArrays(3, innerVAO);
    glGenBuffers(3, innerVBO);

    std::vector<float>* inners[3] = { &inner1, &inner2, &inner3 };

    for(int i = 0; i < 3; i++)
    {
        glBindVertexArray(innerVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, innerVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, inners[i]->size()*sizeof(float), inners[i]->data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
    }

    // ===== Grid Lines =====
    GLuint gridVAO[8], gridVBO[8];
    glGenVertexArrays(8, gridVAO);
    glGenBuffers(8, gridVBO);

    for(int i = 0; i < 8; i++)
    {
        float angle = i * (M_PI / 4.0f);
        float line[6] = {0,0,0, cos(angle)*0.5f, sin(angle)*0.5f,0};

        glBindVertexArray(gridVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
    }

    // ===== Ray =====
    GLuint rayVAO, rayVBO;
    glGenVertexArrays(1, &rayVAO);
    glGenBuffers(1, &rayVBO);

    glBindVertexArray(rayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
    glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    // ===== Wave =====
    GLuint waveVAO, waveVBO;
    glGenVertexArrays(1, &waveVAO);
    glGenBuffers(1, &waveVBO);

    glBindVertexArray(waveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waveVBO);
    glBufferData(GL_ARRAY_BUFFER, CIRCLE_POINTS*3*sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    while(!glfwWindowShouldClose(window))
    {
        rotationAngle += 0.02f;
        if(rotationAngle > 2*M_PI) rotationAngle -= 2*M_PI;

        if(showWave)
        {
            waveRadius += 0.004f;

            float diff = fabs(rotationAngle - waveAngle);
            if(diff > M_PI) diff = 2*M_PI - diff;

            if(diff < 0.04f)
            {
                waveAlpha = 1.0f;
                detected = true;
            }
            else
                waveAlpha -= 0.01f;

            if(waveAlpha <= 0.0f)
                showWave = false;
        }

        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);

        // Main Circle
        glBindVertexArray(radarVAO);
        glUniform4f(colorLoc,0,1,0,1);
        glDrawArrays(GL_LINE_LOOP,0,CIRCLE_POINTS);

        // Inner Circles
        for(int i=0;i<3;i++)
        {
            glBindVertexArray(innerVAO[i]);
            glUniform4f(colorLoc,0,0.6f,0,0.6f);
            glDrawArrays(GL_LINE_LOOP,0,CIRCLE_POINTS);
        }

        // Grid
        for(int i=0;i<8;i++)
        {
            glBindVertexArray(gridVAO[i]);
            glUniform4f(colorLoc,0,0.5f,0,0.4f);
            glDrawArrays(GL_LINES,0,2);
        }

        // Glow Ray
        for(int i=-2;i<=2;i++)
        {
            float offset = i*0.01f;
            float ray[6] = {0,0,0, cos(rotationAngle+offset)*0.5f,
                            sin(rotationAngle+offset)*0.5f,0};

            glBindVertexArray(rayVAO);
            glBindBuffer(GL_ARRAY_BUFFER,rayVBO);
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(ray),ray);

            float intensity = 1.0f - fabs(i)*0.2f;
            glUniform4f(colorLoc,0,1,0,intensity);
            glDrawArrays(GL_LINES,0,2);
        }

        // Wave
        if(showWave)
        {
            std::vector<float> waveVertices;
            generateCircle(waveVertices, waveRadius);

            for(int i=0;i<CIRCLE_POINTS;i++)
            {
                waveVertices[i*3] += waveX;
                waveVertices[i*3+1] += waveY;
            }

            glBindVertexArray(waveVAO);
            glBindBuffer(GL_ARRAY_BUFFER,waveVBO);
            glBufferSubData(GL_ARRAY_BUFFER,0,
                waveVertices.size()*sizeof(float),waveVertices.data());

            glUniform4f(colorLoc,0.5f,1.0f,0.5f,waveAlpha);
            glDrawArrays(GL_LINE_LOOP,0,CIRCLE_POINTS);
        }

        // Detected Target Blip
        if(detected)
        {
            std::vector<float> target;
            generateCircle(target, 0.015f);

            for(int i=0;i<CIRCLE_POINTS;i++)
            {
                target[i*3] += waveX;
                target[i*3+1] += waveY;
            }

            glBindBuffer(GL_ARRAY_BUFFER,waveVBO);
            glBufferSubData(GL_ARRAY_BUFFER,0,
                target.size()*sizeof(float),target.data());

            glUniform4f(colorLoc,0,1,0,1);
            glDrawArrays(GL_TRIANGLE_FAN,0,CIRCLE_POINTS);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

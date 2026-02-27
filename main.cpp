#include <iostream>
#include <cmath>              // لإجراء العمليات الحسابية (sin, cos)
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// إعدادات النافذة
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ====================================
// Vertex Shader للرادار
// ====================================
const char* vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"       // استقبال نقاط XY للرسم
"uniform float uRotation;\n"                 // زاوية الدوران للشعاع
"uniform float uXOffset;\n"                  // تحريك الرادار على X
"void main()\n"
"{\n"
"   float cosR = cos(uRotation);\n"
"   float sinR = sin(uRotation);\n"
"   vec2 rotatedPos = vec2(aPos.x * cosR - aPos.y * sinR, aPos.x * sinR + aPos.y * cosR);\n"
"   gl_Position = vec4(rotatedPos.x + uXOffset, rotatedPos.y, 0.0, 1.0);\n"
"}\0";

// ====================================
// Fragment Shader للرادار
// ====================================
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 uColor;\n"   // اللون النهائي للعنصر
"void main()\n"
"{\n"
"   FragColor = uColor;\n"
"}\0";

// دالة تغيير حجم النافذة
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// معالجة المدخلات
void processInput(GLFWwindow* window, float &xOffset, float &wireframe)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // التحكم بالـ Wireframe/FILL
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wireframe = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) wireframe = 0.0f;

    // تحريك الرادار على محور X
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) xOffset -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) xOffset += 0.01f;
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
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- تهيئة GLEW ---
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW\n";
        return -1;
    }

    // --- بناء وتجميع الشيدر ---
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- إعداد بيانات الرادار ---
    // دائرة الرادار
    const int segments = 100;
    float radarVertices[segments*2]; // x,y لكل نقطة
    for(int i=0;i<segments;i++)
    {
        float angle = 2.0f * M_PI * i / segments;
        radarVertices[i*2] = cos(angle)*0.8f;
        radarVertices[i*2+1] = sin(angle)*0.8f;
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(radarVertices),radarVertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    // --- تفعيل ميزات OpenGL ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float rotation = 0.0f;
    float xOffset = 0.0f;
    float wireframe = 0.0f; // 0 = fill, 1 = line

    // --- حلقة الرسم ---
    while(!glfwWindowShouldClose(window))
    {
        processInput(window, xOffset, wireframe);

        // تبديل Wireframe/FILL
        if(wireframe>0.5f) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // دائرة الرادار باللون الأخضر فاتح
        int colorLoc = glGetUniformLocation(shaderProgram,"uColor");
        glUniform4f(colorLoc,0.0f,1.0f,0.0f,0.5f);
        int rotLoc = glGetUniformLocation(shaderProgram,"uRotation");
        glUniform1f(rotLoc,0.0f);
        int xLoc = glGetUniformLocation(shaderProgram,"uXOffset");
        glUniform1f(xLoc, xOffset);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP,0,segments);

        // شعاع الرادار (متحرك)
        rotation += 0.01f;
        if(rotation>2*M_PI) rotation -= 2*M_PI;

        glUniform1f(rotLoc,rotation);
        glUniform4f(colorLoc,0.0f,1.0f,0.0f,0.8f); // أكثر سطوعًا
        glDrawArrays(GL_LINES,0,2); // نرسم أول نقطتين للشعاع

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

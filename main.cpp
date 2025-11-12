#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include<ctime>

#include <iostream>

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform vec2 u_Offset;

    void main()
    {
        gl_Position = vec4(aPos.x + u_Offset.x, aPos.y + u_Offset.y, aPos.z, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 u_Color;

    void main()
    {
        FragColor = vec4(u_Color, 1.0f);
    }
)glsl";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void checkShaderCompilation(unsigned int shader) {
    int success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void checkProgramLinking(unsigned int program) {
    int success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

struct Ball {
    float x,y;
    float vel_x, vel_y;
    float radius;
    float red,green,blue;

    Ball(float r, float start_x, float start_y, float start_vel_x = 0.0f, float start_vel_y = 0.0f) {
        radius = r;
        x = start_x;
        y = start_y;
        vel_x = start_vel_x;
        vel_y = start_vel_y;

        red = 0.5f + ((float)rand() / (float)RAND_MAX) * 0.5f;
        blue = 0.5f + ((float)rand() / (float)RAND_MAX) * 0.5f;
        green = 0.5f + ((float)rand() / (float)RAND_MAX) * 0.5f;
    }
};

void updateBallPhysics(Ball& ball, float deltaTime) {
    float gravity = -1.0f;
    float damping = -0.9f;
    float container_radius = 1.0f;

    ball.vel_y += gravity * deltaTime;

    ball.x += ball.vel_x * deltaTime;
    ball.y += ball.vel_y * deltaTime;

    float dist_sq = ball.x * ball.x + ball.y * ball.y;

    float max_dist = container_radius - ball.radius;
    float max_dist_sq = max_dist * max_dist;

    if (dist_sq > max_dist_sq) {
        float dist = std::sqrt(dist_sq);

        float n_x = ball.x/dist;
        float n_y = ball.y/dist;

        ball.x = n_x * max_dist;
        ball.y = n_y * max_dist;

        float dot = ball.vel_x * n_x + ball.vel_y * n_y;

        float v_normal_x = dot * n_x;
        float v_normal_y = dot * n_y;
        float v_tangent_x = ball.vel_x - v_normal_x;
        float v_tangent_y = ball.vel_y - v_normal_y;

        v_normal_x *= damping;
        v_normal_y *= damping;

        ball.vel_x = v_tangent_x + v_normal_x;
        ball.vel_y = v_tangent_y + v_normal_y;
    }

    // if (ball.y - ball.radius < -1.0f) {
    //     ball.y = -1.0f + ball.radius;
    //     ball.vel_y *= damping;
    // }
    //
    // if (ball.y + ball.radius > 1.0f) {
    //     ball.y = 1.0f - ball.radius;
    //     ball.vel_y *= damping;
    // }
    //
    // if (ball.x - ball.radius > 1.0f) {
    //     ball.x = 1.0f - ball.radius;
    //     ball.vel_x *= damping;
    // }
    //
    // if (ball.x - ball.radius < -1.0f) {
    //     ball.x = -1.0f + ball.radius;
    //     ball.vel_x *= damping;
    // }
}

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialise GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Balls", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompilation(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLinking(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //Making the circle
    float cx = 0.0f;
    float cy = 0.0f;
    float r = 0.1f;
    int num_segments = 50;

    std::vector<Ball> balls;

    // balls.push_back(Ball(0.1f, 0.0f, 0.5f));
    // balls.push_back(Ball(0.1f, -0.5f, 0.8f, 0.5f, 0.0f));
    // balls.push_back(Ball(0.1f, 0.5f, 0.2f, -0.3f, 0.5f));

    for (int i = 0; i < 15; i++) {
        float randomxPos = (rand() / (float)RAND_MAX) -0.5f;
        float randomyPos = (rand() / (float)RAND_MAX) -0.5f;
        float randomxVel = (rand() / (float)RAND_MAX) -0.5f;
        float randomyVel = (rand() / (float)RAND_MAX) -0.5f;

        balls.push_back(Ball(r, randomxPos, randomyPos, randomxVel, randomyVel));
    }

    float lastFrameTime = 0.0f;

    int offsetLocation = glGetUniformLocation(shaderProgram, "u_Offset");
    int colorLocation = glGetUniformLocation(shaderProgram, "u_Color");

    std::vector<float> vertices;

    vertices.push_back(cx); //x
    vertices.push_back(cy); //y
    vertices.push_back(0.0f); //z

    for (int i = 0; i <= num_segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
    }

    int totalVertices = vertices.size() / 3;
    std::cout << "Total vertices to draw: " << totalVertices << std::endl;

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    while (!glfwWindowShouldClose(window)) {

        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for (Ball& ball : balls) {
            updateBallPhysics(ball, deltaTime);

            glUniform2f(offsetLocation, ball.x, ball.y);

            glUniform3f(colorLocation, ball.red, ball.green, ball.blue);

            glDrawArrays(GL_TRIANGLE_FAN, 0, totalVertices);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
//
//  main.cpp
//  Simple 3D Classroom
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "basic_camera.h"

#include <iostream>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void drawBox(Shader& shader, unsigned int VAO, glm::vec3 position, glm::vec3 size, glm::vec3 color);
void drawClassroom(Shader& shader, unsigned int VAO);
void drawTableAndChair(Shader& shader, unsigned int VAO);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// modelling transform variables from your original code
float rotateAngle_X = 45.0;
float rotateAngle_Y = 45.0;
float rotateAngle_Z = 45.0;
float rotateAxis_X = 0.0;
float rotateAxis_Y = 0.0;
float rotateAxis_Z = 1.0;
float translate_X = 0.0;
float translate_Y = 0.0;
float translate_Z = 0.0;
float scale_X = 1.0;
float scale_Y = 1.0;
float scale_Z = 1.0;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 6.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Basic camera placed so classroom wall, board, table and chair are visible
float eyeX = 0.0f;
float eyeY = 2.2f;
float eyeZ = 6.0f;

float lookAtX = 0.0f;
float lookAtY = 1.1f;
float lookAtZ = -1.5f;

glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simple 3D Classroom", NULL, NULL);

    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // build and compile shader
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");

    /*
        Centered cube vertices.

        This cube goes from -0.5 to +0.5 in X, Y and Z.
        Because it is centered, it is easier to place objects exactly
        where we want them in the classroom.
    */
    float cube_vertices[] = {
        // positions only
        -0.5f, -0.5f, -0.5f,   // 0
         0.5f, -0.5f, -0.5f,   // 1
         0.5f,  0.5f, -0.5f,   // 2
        -0.5f,  0.5f, -0.5f,   // 3

        -0.5f, -0.5f,  0.5f,   // 4
         0.5f, -0.5f,  0.5f,   // 5
         0.5f,  0.5f,  0.5f,   // 6
        -0.5f,  0.5f,  0.5f    // 7
    };

    unsigned int cube_indices[] = {
        // back face
        0, 1, 2,
        2, 3, 0,

        // front face
        4, 5, 6,
        6, 7, 4,

        // left face
        0, 4, 7,
        7, 3, 0,

        // right face
        1, 5, 6,
        6, 2, 1,

        // top face
        3, 2, 6,
        6, 7, 3,

        // bottom face
        0, 1, 5,
        5, 4, 0
    };

    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDisableVertexAttribArray(1);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.45f, 0.60f, 0.70f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // projection
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            100.0f
        );

        ourShader.setMat4("projection", projection);

        // view
        glm::mat4 view = basic_camera.createViewMatrix();
        ourShader.setMat4("view", view);

        // draw classroom wall, board and floor
        drawClassroom(ourShader, VAO);

        // draw table and chair in the middle
        drawTableAndChair(ourShader, VAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

/*
    Draw one rectangular box.

    position = center position of the box
    size     = width, height, depth
    color    = RGB color
*/
void drawBox(Shader& shader, unsigned int VAO, glm::vec3 position, glm::vec3 size, glm::vec3 color)
{
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);
    model = glm::scale(model, size);

    shader.setMat4("model", model);

    // constant color for this object
    glVertexAttrib3f(1, color.r, color.g, color.b);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

/*
    Classroom base:
    - Floor (expanded to 14 wide x 12 deep)
    - Front wall (shifted back to match new floor edge at Z = -6)
    - Blackboard on wall
*/
void drawClassroom(Shader& shader, unsigned int VAO)
{
    // colors
    glm::vec3 floorColor = glm::vec3(0.55f, 0.48f, 0.38f);
    glm::vec3 wallColor = glm::vec3(0.82f, 0.80f, 0.72f);

    // Change this to true if you want a whiteboard instead of blackboard
    bool useWhiteboard = false;

    glm::vec3 boardColor;

    if (useWhiteboard)
        boardColor = glm::vec3(0.92f, 0.92f, 0.88f);   // whiteboard
    else
        boardColor = glm::vec3(0.02f, 0.08f, 0.04f);   // blackboard / dark green board

    glm::vec3 boardFrameColor = glm::vec3(0.35f, 0.22f, 0.10f);
    glm::vec3 trayColor = glm::vec3(0.60f, 0.60f, 0.60f);

    // floor — expanded from 8x7 to 14x12
    // X: -7 to +7     Z: -6 to +6
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, -0.05f, 0.0f),
        glm::vec3(14.0f, 0.10f, 12.0f),
        floorColor
    );

    // front wall — shifted back to Z = -6.0 to sit on the new floor edge
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, 2.0f, -6.0f),
        glm::vec3(14.0f, 4.0f, 0.12f),
        wallColor
    );

    // blackboard / whiteboard
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, 2.35f, -5.90f),
        glm::vec3(4.5f, 1.55f, 0.08f),
        boardColor
    );

    // top frame
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, 3.15f, -5.84f),
        glm::vec3(4.75f, 0.08f, 0.10f),
        boardFrameColor
    );

    // bottom frame
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, 1.55f, -5.84f),
        glm::vec3(4.75f, 0.08f, 0.10f),
        boardFrameColor
    );

    // left frame
    drawBox(
        shader,
        VAO,
        glm::vec3(-2.28f, 2.35f, -5.84f),
        glm::vec3(0.08f, 1.65f, 0.10f),
        boardFrameColor
    );

    // right frame
    drawBox(
        shader,
        VAO,
        glm::vec3(2.28f, 2.35f, -5.84f),
        glm::vec3(0.08f, 1.65f, 0.10f),
        boardFrameColor
    );

    // small chalk / marker tray
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, 1.42f, -5.75f),
        glm::vec3(4.2f, 0.08f, 0.18f),
        trayColor
    );
}

/*
    Table and chair:
    - Kept in the middle of the room
    - Chair is behind the table
    - Both face the front wall / board
*/
void drawTableAndChair(Shader& shader, unsigned int VAO)
{
    // colors
    glm::vec3 tableTopColor = glm::vec3(0.45f, 0.25f, 0.10f);
    glm::vec3 tableLegColor = glm::vec3(0.25f, 0.14f, 0.06f);

    glm::vec3 chairSeatColor = glm::vec3(0.30f, 0.16f, 0.08f);
    glm::vec3 chairLegColor = glm::vec3(0.20f, 0.10f, 0.04f);

    // -------------------------
    // TABLE IN THE MIDDLE
    // -------------------------

    // table top
    drawBox(
        shader,
        VAO,
        glm::vec3(0.0f, 1.15f, 0.0f),
        glm::vec3(2.0f, 0.20f, 1.35f),
        tableTopColor
    );

    // table legs
    drawBox(
        shader,
        VAO,
        glm::vec3(-0.85f, 0.55f, -0.55f),
        glm::vec3(0.16f, 1.10f, 0.16f),
        tableLegColor
    );

    drawBox(
        shader,
        VAO,
        glm::vec3(0.85f, 0.55f, -0.55f),
        glm::vec3(0.16f, 1.10f, 0.16f),
        tableLegColor
    );

    drawBox(
        shader,
        VAO,
        glm::vec3(-0.85f, 0.55f, 0.55f),
        glm::vec3(0.16f, 1.10f, 0.16f),
        tableLegColor
    );

    drawBox(
        shader,
        VAO,
        glm::vec3(0.85f, 0.55f, 0.55f),
        glm::vec3(0.16f, 1.10f, 0.16f),
        tableLegColor
    );

    // -------------------------
    // CHAIR IN FRONT OF BOARD
    // -------------------------

    // chair seat
    drawBox(shader, VAO, glm::vec3(0.0f, 0.62f, -1.4f), glm::vec3(1.0f, 0.22f, 0.95f), chairSeatColor);

    // front legs
    drawBox(shader, VAO, glm::vec3(-0.38f, 0.28f, -1.05f), glm::vec3(0.14f, 0.56f, 0.14f), chairLegColor);
    drawBox(shader, VAO, glm::vec3(0.38f, 0.28f, -1.05f), glm::vec3(0.14f, 0.56f, 0.14f), chairLegColor);

    // back legs
    drawBox(shader, VAO, glm::vec3(-0.38f, 0.28f, -1.75f), glm::vec3(0.14f, 0.56f, 0.14f), chairLegColor);
    drawBox(shader, VAO, glm::vec3(0.38f, 0.28f, -1.75f), glm::vec3(0.14f, 0.56f, 0.14f), chairLegColor);

    // backrest
    drawBox(shader, VAO, glm::vec3(0.0f, 1.15f, -1.90f), glm::vec3(1.0f, 1.15f, 0.16f), chairSeatColor);
}

// process all input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (rotateAxis_X) rotateAngle_X -= 1;
        else if (rotateAxis_Y) rotateAngle_Y -= 1;
        else rotateAngle_Z -= 1;
    }

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) translate_Y += 0.01;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) translate_Y -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) translate_X += 0.01;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) translate_X -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) translate_Z += 0.01;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) translate_Z -= 0.01;

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) scale_X += 0.01;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) scale_X -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) scale_Y += 0.01;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) scale_Y -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) scale_Z += 0.01;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) scale_Z -= 0.01;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        rotateAngle_X += 1;
        rotateAxis_X = 1.0;
        rotateAxis_Y = 0.0;
        rotateAxis_Z = 0.0;
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        rotateAngle_Y += 1;
        rotateAxis_X = 0.0;
        rotateAxis_Y = 1.0;
        rotateAxis_Z = 0.0;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        rotateAngle_Z += 1;
        rotateAxis_X = 0.0;
        rotateAxis_Y = 0.0;
        rotateAxis_Z = 1.0;
    }

    // Basic camera movement keys
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        eyeX += 2.5f * deltaTime;
        basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        eyeX -= 2.5f * deltaTime;
        basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        eyeZ += 2.5f * deltaTime;
        basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        eyeZ -= 2.5f * deltaTime;
        basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyeY += 2.5f * deltaTime;
        basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        eyeY -= 2.5f * deltaTime;
        basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        lookAtX += 2.5f * deltaTime;
        basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        lookAtX -= 2.5f * deltaTime;
        basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        lookAtY += 2.5f * deltaTime;
        basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        lookAtY -= 2.5f * deltaTime;
        basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        lookAtZ += 2.5f * deltaTime;
        basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
    {
        lookAtZ -= 2.5f * deltaTime;
        basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
    {
        basic_camera.changeViewUpVector(glm::vec3(1.0f, 0.0f, 0.0f));
    }

    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
    {
        basic_camera.changeViewUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
    {
        basic_camera.changeViewUpVector(glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

// framebuffer callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// mouse callback
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
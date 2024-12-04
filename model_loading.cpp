#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "filesystem.h"
#include "model.h"
#include "shader_m.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void updateCamera();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Model *ourModel;
glm::vec3 cameraOffset(0.0f, 3.0f, 10.0f); // Adjust for desired fixed distance and height

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");

    // load models
    // -----------
    // Model ourModel(FileSystem::getPath("oiiaioooooiai_cat/oiiaioooooiai_cat.obj"));
    Model *newModel = new Model(FileSystem::getPath("backpack/backpack.obj"));
    ourModel = new Model(FileSystem::getPath("oiiaioooooiai_cat/oiiaioooooiai_cat.obj"));
    ourModel->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    newModel->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // Timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input handling
        processInput(window);

        // Update camera to follow ourModel
        camera.updateCameraVectors(ourModel->GetPosition(), 10.0f);

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // Projection and View Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render ourModel
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ourModel->GetPosition());
        ourShader.setMat4("model", model);
        ourModel->Draw(ourShader);

        // Render newModel independently
        glm::mat4 newModelMatrix = glm::mat4(1.0f);
        newModelMatrix = glm::translate(newModelMatrix, newModel->GetPosition());
        ourShader.setMat4("model", newModelMatrix);
        newModel->Draw(ourShader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    delete ourModel;
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 moveDirection(0.0f);
    float speed = 5.0f * deltaTime; // Adjust speed as necessary

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveDirection += camera.Front; // Move forward relative to the camera
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDirection -= camera.Front; // Move backward relative to the camera
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDirection -= camera.Right; // Move left relative to the camera
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDirection += camera.Right; // Move right relative to the camera

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection); // Normalize to ensure consistent movement speed
        ourModel->SetPosition(ourModel->GetPosition() + moveDirection * speed);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    float distance = 5.0f; // Set the desired distance from the model
    // Pass model position and distance to ProcessMouseMovement
    camera.ProcessMouseMovement(xoffset, yoffset, ourModel->GetPosition(), distance);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
void updateCamera() {
    glm::vec3 modelPosition = ourModel->GetPosition();
    float distance = 10.0f; // Desired fixed distance from the model
    camera.Position = modelPosition + cameraOffset;
    camera.Front = glm::normalize(modelPosition - camera.Position);
    camera.updateCameraVectors();
}
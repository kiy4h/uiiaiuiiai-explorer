#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "filesystem.h"
#include "model.h"
#include "shader.h"
#include "skybox.h"
#include "terrain.h"

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
int isRotate;

GLuint terrainVAO;
Model *ourModel;
Terrain *terrain;
glm::vec3 cameraOffset(0.0f, 3.0f, 10.0f); // Adjust for desired fixed distance and height

glm::vec3 lightPos(5000.0f, 9000.0f, 2000.0f);  // Position of light source
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // Light color (white light)
glm::vec3 objectColor(1.0f, 1.0f, 1.0f); // Color of the object

int main() {

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    // Request OpenGL 3.3 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    });

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
    Shader ourShader("shaders/1.model_loading.vs", "shaders/1.model_loading.fs");
    Shader lightingShader("shaders/lighting.vs", "shaders/lighting.fs");
    Shader terrainShader("shaders/terrain.vs", "shaders/terrain.fs");
    

    // load models
    // -----------
    // Model ourModel(FileSystem::getPath("oiiaioooooiai_cat/oiiaioooooiai_cat.obj"));
    Model *newModel = new Model(FileSystem::getPath("models/backpack/backpack.obj"));
    ourModel = new Model(FileSystem::getPath("models/oiiaioooooiai_cat/oiiaioooooiai_cat.obj"));
    ourModel->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    newModel->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    // Create the terrain using the heightmap
    terrain = new Terrain("height-map.png", 10.0f, 256, 256);
    terrain->loadTexture("images/grass.jpg"); // Load the texture for the terrain

    terrain->addModel("grass", "models/grass/grass.obj");       // Load the model for the grass
    terrain->generateObjects(1000, "grass", 0.0f, 10.0f, 0.5f); // Grass near flat terrain

    std::vector<std::string> faces = {
        "images/skybox/right.jpg", "images/skybox/left.jpg", "images/skybox/top.jpg",
        "images/skybox/bottom.jpg", "images/skybox/front.jpg", "images/skybox/back.jpg"};
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
    Skybox skybox(faces, skyboxShader);

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    isRotate = 0;

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
        

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 vp = projection * view;

        // ** Render terrain **
        terrainShader.use();
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("projection", projection);
        terrainShader.setVec3("viewPos", camera.Position);
        // terrainShader.setVec3("lightPos", lightPos);
        // terrainShader.setVec3("lightColor", lightColor);

        // set light
        lightingShader.use();
        // Set light position dan kamera (view position)
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setVec3("objectColor", objectColor);
        lightingShader.setVec3("lightColor", lightColor);
        lightingShader.setFloat("shininess", 10.0f);

        glm::mat4 terrainModel = glm::mat4(1.0f); // Adjust position/scale as needed
        lightingShader.setMat4("model", terrainModel);
        // terrainShader.setMat4("model", terrainModel);
        terrain->render(lightingShader, vp);

        // ** Render ourModel **
        ourShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ourModel->GetPosition());
        model = glm::rotate(model, glm::radians(ourModel->GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
        // lightingShader.setMat4("model", model);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);
        ourModel->Draw(ourShader);

        // ** Render objects **
        ourShader.use();
        terrain->renderObjects(ourShader, vp);

        // ** Render the skybox **
        skybox.render(camera.GetViewMatrix(), projection, camera);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    delete terrain;
    delete ourModel;
    return 0;
}

// Simple linear interpolation for floats
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 moveDirection(0.0f);
    float speed = 5.0f * deltaTime; // Adjust speed as needed

    // Flatten the camera's Front and Right vectors for movement on the XZ plane
    glm::vec3 frontXZ = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    glm::vec3 rightXZ = glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z));

    // Add/subtract movement directions based on input
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveDirection += frontXZ; // Forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDirection -= frontXZ; // Backward
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDirection -= rightXZ; // Left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDirection += rightXZ; // Right
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) isRotate = isRotate ? 0 : 1;

    // Normalize the movement direction if there's input
    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection); // Ensure uniform speed

        glm::vec3 currentPosition = ourModel->GetPosition();
        glm::vec3 newPosition = currentPosition + moveDirection * speed;

        // Keep the model on top of the terrain
        float terrainHeight = terrain->getHeightAt(newPosition.x, newPosition.z);

        // Smoothly interpolate the Y position towards the target height
        float smoothFactor = 0.1f; // Adjust this value for more/less smoothness
        newPosition.y = glm::mix(currentPosition.y, terrainHeight, smoothFactor);

        // Clamp position to stay within terrain bounds
        newPosition.x = glm::clamp(newPosition.x, 0.0f, (float)(terrain->getWidth() - 1));
        newPosition.z = glm::clamp(newPosition.z, 0.0f, (float)(terrain->getHeight() - 1));
        // Set the model's new position
        ourModel->SetPosition(newPosition);

        // Calculate target rotation (yaw angle) based on moveDirection
        float targetYaw = glm::degrees(glm::atan(moveDirection.z, moveDirection.x));
        // Get the current yaw angle of the model
        float currentYaw = ourModel->GetRotation().y;
        if (isRotate) {
            // Ensure angles are in the range [0, 360)
            targetYaw = fmod(targetYaw + 360.0f, 360.0f);
            currentYaw = fmod(currentYaw + 360.0f, 360.0f);

            // Determine the shortest direction to rotate
            float rotationSpeed = 400.0f * deltaTime; // Adjust speed (degrees per second)
            currentYaw += rotationSpeed;

            // Wrap currentYaw back to [0, 360) if needed
            currentYaw = fmod(currentYaw + 360.0f, 360.0f);

            // Update model's rotation (yaw only)
            ourModel->SetRotation(glm::vec3(0.0f, currentYaw, 0.0f));

            // std::cout << "Current Yaw: " << currentYaw
            //           << ", Target Yaw: " << targetYaw
            //           << ", Rotation Speed: " << rotationSpeed << std::endl;
        } else {
            // Smoothly interpolate (lerp) between current and target yaw angles
            float smoothFactor = 0.1f; // Adjust for more/less smoothness
            float newYaw = glm::mix(currentYaw, -targetYaw + 90, smoothFactor);

            // Update model's rotation (yaw only)
            ourModel->SetRotation(glm::vec3(0.0f, newYaw, 0.0f));
        }
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
    // camera.updateCameraVectors();
}
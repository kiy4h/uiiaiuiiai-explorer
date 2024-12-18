
#define SDL_MAIN_HANDLED
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "game_controller.h"
#include "model.h"
#include "popup.h"
#include "shader.h"
#include "skybox.h"
#include "sound_manager.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera *camera = new Camera(glm::vec3(0.0f, 5.0f, 10.0f)); // Example initial position for the camera
float cameraDistance = 10.0f;                              // Set the desired distance from the model
float yOffset = 0.0f;                                      // Vertical offset to keep the camera pointing above the player
float camxoffset = 0, camyoffset = 0;
bool firstMouse = true;

Model *player;
Terrain *terrain;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
glm::vec3 cameraOffset(0.0f, 3.0f, 10.0f);  // Adjust for desired fixed distance and height
glm::vec3 lightPos(100.0f, 100.0f, 100.0f); // Position of light source
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);     // Light color (white light)

int main() {

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    // Request OpenGL 3.3 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Game", NULL, NULL);
    if (window == nullptr) {
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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // build and compile shaders
    // -------------------------
    Shader playerShader("shaders/model.vs", "shaders/model.fs");
    Shader terrainShader("shaders/terrain_test.vs", "shaders/terrain_test.fs");
    Shader collectibleShader("shaders/collectible.vs", "shaders/collectible.fs");
    Shader overlayShader("shaders/overlay.vs", "shaders/overlay.fs");
    cout << "Shaders compiled!" << endl;

    // Initialize sound manager
    SoundManager soundManager;
    // Load background music
    soundManager.loadBGM("audio/maxwell-bgm.mp3", "game");
    soundManager.loadBGM("audio/yippee.mp3", "win");
    soundManager.loadBGM("audio/sad-moment.mp3", "lose");
    // Load sound effects
    soundManager.loadSoundEffect("collect", "audio/oiiiiiai beat drop.mp3");
    soundManager.loadSoundEffect("footstep", "audio/oiiaioiiiai-mini.mp3");
    cout << "Sound manager initialized!" << endl;

    // Initialize terrain
    terrain = new Terrain("images/height-map.png", 10.0f, 256, 256);
    // Initialize player
    player = new Model(FileSystem::getPath("models/oiiaioooooiai_cat/oiiaioooooiai_cat.obj"));

    // collectibles: Create a collectible manager and add collectibles
    CollectibleManager collectibleManager(soundManager);
    GameController gameController(window, camera, collectibleManager, soundManager, terrain, player);

    // Initialize the game
    gameController.initGame();

    std::vector<std::string> faces = {
        "images/skybox/right.jpg", "images/skybox/left.jpg", "images/skybox/top.jpg",
        "images/skybox/bottom.jpg", "images/skybox/front.jpg", "images/skybox/back.jpg"};
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
    Skybox skybox(faces, skyboxShader);
    cout << "Skybox initialized!" << endl;

    // Load font
    TextRenderer textRenderer("FredokaOne-Regular.ttf", 28);
    Shader textShader("shaders/text.vs", "shaders/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
    textShader.use();
    textShader.setMat4("projection", projection);
    cout << "Text renderer initialized!" << endl;

    // Create popups
    Popup winPopup("You Win!", glm::vec3(1.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    Popup losePopup("You Lose!", glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    cout << "Popups initialized!" << endl;

    // Play background music
    soundManager.changeBGM("game");
    soundManager.playBGM();

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    cout << "Game started!" << endl;

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // Input handling: Check for ESC key to exit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Check game state and show popups if needed
        if (gameController.getGameState() == GameState::Won) {
            winPopup.show();
            soundManager.stopAllSoundEffects();
            soundManager.changeBGM("win");
            gameController.setGameState(GameState::AudioPlayed);
        } else if (gameController.getGameState() == GameState::Lost) {
            losePopup.show();
            soundManager.stopAllSoundEffects();
            soundManager.changeBGM("lose");
            gameController.setGameState(GameState::AudioPlayed);
        }

        // Restart the game when necessary
        if ((winPopup.isVisible() || losePopup.isVisible()) && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            gameController.restartGame();
            winPopup.hide();
            losePopup.hide();
        }

        // Update game state
        gameController.update();

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate dynamic yOffset based on the mouse's vertical movement
        float dynamicYOffset = glm::clamp(glm::abs(camyoffset) * 0.05f, 0.0f, 5.0f); // Scale dynamically, max offset = 5.0f

        // Check if the camera "collides" with the terrain
        glm::vec3 camPosition = camera->Position;
        float terrainHeight = terrain->getHeightAt(camPosition.x, camPosition.z);
        bool applyYOffset = camPosition.y <= terrainHeight; // Apply offset only if the camera is close to the terrain
        // Update the camera vectors
        float distance = 10.0f; // Desired distance from the player
        camera->updateCameraVectors(player->GetPosition(), distance, terrain, dynamicYOffset, applyYOffset);

        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 vp = projection * view;

        // ** Render the skybox **
        skybox.render(camera->GetViewMatrix(), projection, camera);

        // ** Render player **
        playerShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, player->GetPosition());
        model = glm::rotate(model, glm::radians(player->GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
        playerShader.setMat4("projection", projection);
        playerShader.setMat4("view", view);
        playerShader.setMat4("model", model);
        player->Draw(playerShader);

        // Render collectibles
        collectibleShader.use();
        collectibleManager.renderAll(collectibleShader, vp);

        // ** Render terrain **
        terrainShader.use();
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("projection", projection);
        terrainShader.setVec3("lightPos", lightPos);
        terrainShader.setVec3("viewPos", camera->Position);
        terrainShader.setVec3("lightColor", lightColor);
        glm::mat4 terrainModel = glm::mat4(1.0f); // Identity matrix for no transformation
        terrainShader.setMat4("model", terrainModel);
        terrain->render(terrainShader, vp); // Render the terrain

        // ** Render objects **
        terrain->renderObjects(playerShader, vp);
        // Render the scoreboard
        std::string scoreText = "Score: " + std::to_string(gameController.getCollectedCount()) + "/" + std::to_string(collectibleManager.getTotalCount());
        textRenderer.RenderText(textShader, scoreText, SCR_WIDTH - 150.0f, SCR_HEIGHT - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)); // Top-right corner

        // Render timer
        float countdownTimer = gameController.getCountdownTimer();
        std::string timerText = "Time: " + std::to_string(static_cast<int>(countdownTimer / 60.0f)) + "m " + std::to_string(static_cast<int>(countdownTimer) % 60) + "s";
        textRenderer.RenderText(textShader, timerText, 20.0f, SCR_HEIGHT - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Render popups
        if (winPopup.isVisible()) {
            winPopup.render(textRenderer, overlayShader, textShader, SCR_WIDTH, SCR_HEIGHT);
        }
        if (losePopup.isVisible()) {
            losePopup.render(textRenderer, overlayShader, textShader, SCR_WIDTH, SCR_HEIGHT);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    delete terrain;
    delete player;
    return 0;
}

// Simple linear interpolation for floats
float lerp(float a, float b, float t) {
    return a + t * (b - a);
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

    // Calculate mouse offsets
    camxoffset = xpos - lastX;
    camyoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    // Clamp mouse offsets to a maximum range for smoother movement
    float maxOffset = 50.0f; // Adjust as needed
    camxoffset = glm::clamp(camxoffset, -maxOffset, maxOffset);
    camyoffset = glm::clamp(camyoffset, -maxOffset, maxOffset);

    // Update last known mouse positions
    lastX = xpos;
    lastY = ypos;

    // Adjust the camera's pitch and yaw based on mouse movement
    camera->ProcessMouseMovement(camxoffset, camyoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(static_cast<float>(yoffset));
}
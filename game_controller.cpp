#include "lib/game_controller.h"

GameController::GameController(GLFWwindow *window, Camera *camera, CollectibleManager &collectibleManager, SoundManager &soundManager, Terrain *terrain, Model *player)
    : window(window), camera(camera), collectibleManager(collectibleManager), soundManager(soundManager), terrain(terrain), player(player), gameState(GameState::Initializing) {
    countdownTimer = 30.0f; // Set timer (e.g., 30 seconds)

    // Optionally set up initial game states or behaviors
    std::cout << "GameController initialized!" << std::endl;
}

void GameController::update() {
    // Timing
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Update game logic
    if (gameState == GameState::Playing) {
        // Input handling
        processInput(window, deltaTime);

        // Update collectibles
        for (auto &collectible : collectibleManager.getCollectibles()) {
            collectible.update(deltaTime);
        }

        // Update countdown timer
        countdownTimer -= deltaTime;
        // Check for player collision with collectibles
        float playerRadius = 1.0f; // Example radius for collision detection
        collectibleManager.checkAllCollisions(player->GetPosition(), playerRadius);
        // Update boost
        updateBoost(deltaTime);

        // Check if all collectibles are collected
        if (collectibleManager.getCollectedCount() == collectibleManager.getTotalCount()) {
            setGameWon(); // Set the game state to "Won"
            return;
        }
        // Check if the timer runs out
        if (countdownTimer <= 0.0f) {
            setGameLost(); // Set the game state to "Lost"
        }
    }
}

bool GameController::hasPlayerWon() const {
    return collectibleManager.getCollectedCount() == collectibleManager.getTotalCount();
}

int GameController::getCollectedCount() const {
    return collectibleManager.getCollectedCount();
}

void GameController::processInput(GLFWwindow *window, float deltaTime) {
    moveDirection = glm::vec3(0.0f);

    // Flatten the camera's Front and Right vectors for movement on the XZ plane
    glm::vec3 frontXZ = glm::normalize(glm::vec3(camera->Front.x, 0.0f, camera->Front.z));
    glm::vec3 rightXZ = glm::normalize(glm::vec3(camera->Right.x, 0.0f, camera->Right.z));

    // Add/subtract movement directions based on input
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveDirection += frontXZ; // Forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDirection -= frontXZ; // Backward
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDirection -= rightXZ; // Left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDirection += rightXZ; // Right

    isMoving = glm::length(moveDirection) > 0.0f;

    // If there's input, update lastMoveDirection and play footstep sound
    if (isMoving) {
        lastMoveDirection = glm::normalize(moveDirection); // Normalize to ensure uniform speed
        if (!isRotate) soundManager.playFootsteps();       // Play footstep sound
    } else {
        if (!isRotate) soundManager.stopFootsteps(); // Stop footstep sound
    }

    // Use lastMoveDirection only if boost is active (isRotate == true)
    glm::vec3 curMoveDirection = (isMoving) ? moveDirection : (isRotate ? lastMoveDirection : glm::vec3(0.0f));

    // Prevent NaN values by ensuring curMoveDirection is valid
    if (glm::length(curMoveDirection) > 0.0f || isRotate) {
        float speed = 5.0f * deltaTime;                      // Adjust speed as needed
        curMoveDirection = glm::normalize(curMoveDirection); // Ensure uniform speed

        glm::vec3 currentPosition = player->GetPosition();
        glm::vec3 newPosition = currentPosition + curMoveDirection * speed * playerSpeed;

        // Keep the model on top of the terrain
        float terrainHeight = terrain->getHeightAt(newPosition.x, newPosition.z);

        // Smoothly interpolate the Y position towards the target height
        float smoothFactor = 0.7f; // Adjust this value for more/less smoothness
        newPosition.y = glm::mix(currentPosition.y, terrainHeight, smoothFactor);

        // Clamp position to stay within terrain bounds
        float terrainOffset = 10.0f; // Adjust for more/less offset
        newPosition.x = glm::clamp(newPosition.x, terrainOffset, (float)(terrain->getWidth() - 1) - terrainOffset);
        newPosition.z = glm::clamp(newPosition.z, terrainOffset, (float)(terrain->getHeight() - 1) - terrainOffset);
        // Set the model's new position
        player->SetPosition(newPosition);

        // Calculate target rotation (yaw angle) based on curMoveDirection
        float targetYaw = glm::degrees(glm::atan(curMoveDirection.z, curMoveDirection.x));
        // Get the current yaw angle of the model
        float currentYaw = player->GetRotation().y;
        if (!isRotate) {
            // Smoothly interpolate (lerp) between current and target yaw angles
            float smoothFactor = 0.3f; // Adjust for more/less smoothness
            float newYaw = glm::mix(currentYaw, -targetYaw + 90 + 360, smoothFactor);

            // Update model's rotation (yaw only)
            player->SetRotation(glm::vec3(0.0f, newYaw, 0.0f));
        }
    }

    // Handle rotation during boost
    if (isRotate) {
        glm::vec3 currentRotation = player->GetRotation();
        currentRotation.y += 400.0f * deltaTime; // Spin at a speed of 400 degrees per second
        player->SetRotation(currentRotation);
    }
}

void GameController::initGame() {
    // Reset game state
    gameState = GameState::Initializing;
    countdownTimer = 60.0f; // Set timer (e.g., 30 seconds)

    // collectibleManager.clear();            // Clear all collectibles
    collectibleManager.setCollectibles(10); // Set the number of collectibles

    terrain->loadTexture("images/grass_green.png");
    terrain->addModel("grasstall", "models/grass_tall/grass_tall.obj");
    terrain->generateObjects(500, "grasstall", 0.0f, 10.0f, 0.5f, 0.001f, 0.007f);
    terrain->addModel("grass", "models/grass/grass.obj");
    terrain->generateObjects(500, "grass", 0.0f, 10.0f, 0.5f, 0.2f, 0.5f);

    terrain->addModel("fern_grass", "models/fern_grass/fern_grass.obj");
    terrain->generateObjects(100, "fern_grass", 0.0f, 10.0f, 0.5f, 0.02f, 0.1f);
    terrain->addModel("bush", "models/bush/shrub.obj");
    terrain->generateObjects(100, "bush", 1.0f, 10.0f, 1.0f, 0.2f, 0.7f);

    terrain->addModel("rock", "models/rock_scan/rock_scan.obj");
    terrain->generateObjects(20, "rock", 2.0f, 20.0f, 1.0f, 0.5f, 1.0f);

    terrain->addModel("pine_tree", "models/pine_tree/pine_tree.obj");
    terrain->generateObjects(50, "pine_tree", 2.0f, 15.0f, 5.0f, 0.01f, 0.03f);
    terrain->addModel("tree", "models/pohon/lowpoly_tree.obj");
    terrain->generateObjects(50, "tree", 2.0f, 15.0f, 5.0f, 3.0f, 5.0f);
    cout << "Terrain objects initialized!" << endl;

    player->SetPosition(glm::vec3(128 / 2, terrain->getHeightAt(128 / 2, 128 / 2), 128 / 2));

    // Initialize collectibles
    float x = 128 / 2, z = 128 / 2;
    float y = terrain->getHeightAt(x, z + 2);
    collectibleManager.addCollectible(glm::vec3(x, y, z + 2), "models/star/star.obj");
    y = terrain->getHeightAt(x, z - 2);
    collectibleManager.addCollectible(glm::vec3(x, y, z - 2), "models/star/star.obj");

    for (int i = 0; i < 20; ++i) {
        x = static_cast<float>(rand() % terrain->getWidth() - 20.0f) + 10;
        z = static_cast<float>(rand() % terrain->getHeight() - 20.0f) + 10;
        y = terrain->getHeightAt(x, z);
        collectibleManager.addCollectible(glm::vec3(x, y, z), "models/star/star.obj");
        std::cout << "Collectibles -- x: " << x << ", z: " << z << std::endl;
    }

    cout << "Collectibles initialized!" << endl;

    lastFrame = glfwGetTime();      // Reset timing
    gameState = GameState::Playing; // Set game state to Playing
    std::cout << "Game initialized!" << std::endl;
}

void GameController::restartGame() {
    // restart game; do not regenerate terrain.
    gameState = GameState::Initializing; // Reset game state
    countdownTimer = 60.0f;              // Set timer (e.g., 30 seconds)
    boostTimer = 0.0f;

    collectibleManager.setCollectibles(10); // Set the number of collectibles

    // Reset player position
    player->SetPosition(glm::vec3(128 / 2, terrain->getHeightAt(128 / 2, 128 / 2), 128 / 2));

    // enable already existing collectibles
    collectibleManager.uncollectAll();

    lastFrame = glfwGetTime(); // Reset timing
    soundManager.changeBGM("game");
    gameState = GameState::Playing; // Set game state to Playing
    cout << "Game restarted!" << endl;
}
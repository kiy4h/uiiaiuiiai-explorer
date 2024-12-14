#include "game_controller.h"
// #include "filesystem.h" // Include the header file for FileSystem
#include <GLFW/glfw3.h> // Include the GLFW header file

GameController::GameController(CollectibleManager &collectibleManager)
    : collectibleManager(collectibleManager) {}

void GameController::update(const glm::vec3 &playerPosition, float playerRadius) {
    collectibleManager.checkAllCollisions(playerPosition, playerRadius);
}

bool GameController::hasPlayerWon() const {
    return collectibleManager.getCollectedCount() == collectibleManager.getTotalCount();
}

int GameController::getCollectedCount() const {
    return collectibleManager.getCollectedCount();
}

void GameController::initGame(
    Terrain *&terrain, Model *&player,
    CollectibleManager &collectibleManager, GameController &gameController,
    float &countdownTimer, bool &gameWon, bool &gameLost, float &lastFrame) {
    // Reset game states
    gameWon = false;
    gameLost = false;
    countdownTimer = 30.0f; // Set timer (e.g., 30 seconds)

    collectibleManager.clear();            // Clear all collectibles
    collectibleManager.setCollectibles(5); // Set the number of collectibles

    terrain->loadTexture("images/grass.jpg");
    terrain->addModel("grass", "models/grass/grass.obj");
    terrain->generateObjects(1000, "grass", 0.0f, 10.0f, 0.5f);
    terrain->addModel("rock", "models/rock_scan/rock_scan.obj");
    terrain->generateObjects(100, "rock", 0.0f, 10.0f, 0.5f);
    terrain->addModel("bush", "models/bush/shrub.obj");
    terrain->generateObjects(1000, "bush", 0.0f, 10.0f, 0.5f);
    terrain->addModel("tree", "models/pohon/lowpoly_tree.obj");
    terrain->generateObjects(500, "tree", 0.0f, 10.0f, 0.5f);

    player->SetPosition(glm::vec3(256 / 2, terrain->getHeightAt(256 / 2, 256 / 2), 256 / 2));

    // Initialize collectibles
    for (int i = 0; i < 5; ++i) {
        float x = static_cast<float>(rand() % terrain->getWidth());
        float z = static_cast<float>(rand() % terrain->getHeight());
        float y = terrain->getHeightAt(x, z);
        collectibleManager.addCollectible(glm::vec3(x, y + 0.25f, z), "models/little_star/little_star.obj");
        std::cout << "Collectibles -- x: " << x << ", z: " << z << std::endl;
    }

    lastFrame = glfwGetTime(); // Reset timing
    std::cout << "Game initialized!" << std::endl;
}

void GameController::restartGame(
    Terrain *&terrain, Model *&player,
    CollectibleManager &collectibleManager, GameController &gameController,
    float &countdownTimer, bool &gameWon, bool &gameLost, float &lastFrame) {
    // restart game; do not regenerate terrain.
    gameWon = false;
    gameLost = false;
    countdownTimer = 30.0f; // Set timer (e.g., 30 seconds)

    // Reset player position
    player->SetPosition(glm::vec3(256 / 2, terrain->getHeightAt(256 / 2, 256 / 2), 256 / 2));

    // enable already existing collectibles
    collectibleManager.uncollectAll();

    lastFrame = glfwGetTime(); // Reset timing
    cout << "Game restarted!" << endl;
}
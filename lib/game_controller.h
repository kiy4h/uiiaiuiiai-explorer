#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "camera.h"
#include "collectibles.h"
#include "filesystem.h"
#include "terrain.h"

enum class GameState {
    Initializing, // Game setup
    Playing,      // Game in progress
    Won,          // Player has won
    Lost,         // Player has lost
    AudioPlayed
};

class GameController {
public:
    GameController(GLFWwindow *window, Camera *camera,
                   CollectibleManager &collectibleManager, SoundManager &soundManager, Terrain *terrain, Model *player);

    bool hasPlayerWon() const;
    int getCollectedCount() const;
    GameState getGameState() const { return gameState; }

    void update();
    void setGameState(GameState state) { gameState = state; }
    void setGameWon() { gameState = GameState::Won; }
    void setGameLost() { gameState = GameState::Lost; }

    // Input processing
    void processInput(GLFWwindow *window, float deltaTime);
    void initGame();
    void restartGame();

    int getCountdownTimer() const { return static_cast<int>(countdownTimer); }
    void updateBoost(float deltaTime) {
        if (collectibleManager.isBoostActive) {
            if (!isRotate) {
                soundManager.playSoundEffect("collect");
                boostTimer = boostDuration;
            }
            collectibleManager.isBoostActive = false;
        }
        if (boostTimer > 0.0f) {
            boostTimer -= deltaTime;

            // Make the player spin (adjust speed as needed)
            isRotate = true;
            playerSpeed = 3.0f;
        } else {
            isRotate = false;
            playerSpeed = 1.0f;
        }
    }

private:
    GLFWwindow *window;
    CollectibleManager &collectibleManager;
    SoundManager &soundManager;
    GameState gameState; // Current state of the game
    Camera *camera;
    Terrain *terrain;
    Model *player;
    bool isRotate = false;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float countdownTimer = 30;
    float playerSpeed = 1;
    glm::vec3 moveDirection = glm::vec3(0.0f);
    glm::vec3 lastMoveDirection = glm::vec3(0.0f); // Default to no movement

    bool isMoving = false;
    float boostTimer = 0.0f;          // Remaining boost time
    const float boostDuration = 6.0f; // Duration of the boost
};

#endif // GAME_CONTROLLER_H

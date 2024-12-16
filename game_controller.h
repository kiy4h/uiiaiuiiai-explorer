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

private:
    GLFWwindow *window;
    CollectibleManager &collectibleManager;
    SoundManager &soundManager;
    GameState gameState; // Current state of the game
    Camera *camera;
    Terrain *terrain;
    Model *player;
    bool isRotate = true;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float countdownTimer = 30;
};

#endif // GAME_CONTROLLER_H

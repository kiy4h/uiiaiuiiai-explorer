#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "collectibles.h"
#include "terrain.h"

class GameController {
public:
    GameController(CollectibleManager &collectibleManager);

    void update(const glm::vec3 &playerPosition, float playerRadius);
    bool hasPlayerWon() const;
    int getCollectedCount() const;

    void initGame(
        Terrain *&terrain, Model *&player,
        CollectibleManager &collectibleManager, GameController &gameController,
        float &countdownTimer, bool &gameWon, bool &gameLost, float &lastFrame);
    void restartGame(
        Terrain *&terrain, Model *&player,
        CollectibleManager &collectibleManager, GameController &gameController,
        float &countdownTimer, bool &gameWon, bool &gameLost, float &lastFrame);

private:
    CollectibleManager &collectibleManager;
};

#endif // GAME_CONTROLLER_H

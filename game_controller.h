#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "collectibles.h"

class GameController {
public:
    GameController(CollectibleManager &collectibleManager);

    void update(const glm::vec3 &playerPosition, float playerRadius);
    bool hasPlayerWon() const;

private:
    CollectibleManager &collectibleManager;
};

#endif // GAME_CONTROLLER_H

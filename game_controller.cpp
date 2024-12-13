#include "game_controller.h"

GameController::GameController(CollectibleManager &collectibleManager)
    : collectibleManager(collectibleManager) {}

void GameController::update(const glm::vec3 &playerPosition, float playerRadius) {
    collectibleManager.checkAllCollisions(playerPosition, playerRadius);
}

bool GameController::hasPlayerWon() const {
    return collectibleManager.getCollectedCount() == collectibleManager.getTotalCount();
}

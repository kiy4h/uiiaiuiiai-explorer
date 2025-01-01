#include "lib/collectibles.h"

Collectible::Collectible(const glm::vec3 &position, const std::string &type, float scale)
    : position(position), currentPos(position), type(type), collected(false), model(type),
      scale(scale), rotationY(static_cast<float>(rand() % 360)),
      bobbingPhase(static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI) {

    // Randomize initial rotation (0 to 360 degrees)
    rotationY = static_cast<float>(rand() % 360);

    // Adjust the initial bobbing offset
    float bobbingSpeed = 2.0f; // Match the bobbing speed in update()
    bobbingOffset = 0.2f * sin(bobbingPhase * bobbingSpeed);
    currentPos = position + glm::vec3(0.0f, bobbingOffset, 0.0f);
}

void Collectible::render(Shader &shader, const glm::mat4 &vp) {
    if (!collected) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, currentPos);                                        // Apply animated position
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));                                      // Apply scaling

        shader.use();
        shader.setInt("texture_diffuse", 0); // Use texture unit 0
        shader.setMat4("model", modelMatrix);
        shader.setMat4("viewProjection", vp);

        // Set emissive color and intensity
        shader.setVec3("emissiveColor", glm::vec3(1.0f, 0.8f, 0.2f)); // Golden glow
        shader.setFloat("emissiveIntensity", 1.0f);

        model.Draw(shader);
    }
}

void Collectible::update(float deltaTime) {
    if (!collected) {
        // Increment rotation angle
        float rotationSpeed = 100.0f; // Degrees per second
        rotationY += rotationSpeed * deltaTime;
        if (rotationY > 360.0f) {
            rotationY -= 360.0f; // Keep rotation within 0-360 degrees
        }

        // Update bobbing offset
        float bobbingSpeed = 2.0f;  // Cycles per second
        float bobbingHeight = 0.2f; // Maximum height offset
        bobbingOffset = bobbingHeight * sin(glfwGetTime() * bobbingSpeed + bobbingPhase);

        // Update current position for rendering
        currentPos = position + glm::vec3(0.0f, bobbingOffset, 0.0f);
    }
}

bool Collectible::checkCollision(const glm::vec3 &playerPosition, float radius) {
    float distance = glm::length(playerPosition - position);
    return !collected && (distance < radius);
}

void CollectibleManager::addCollectible(const glm::vec3 &position, const std::string &type, float scale) {
    collectibles.emplace_back(position + glm::vec3(0.0f, 0.5f, 0.0f), type, scale);
}

void CollectibleManager::renderAll(Shader &shader, const glm::mat4 &vp) {
    for (auto &collectible : collectibles) {
        collectible.render(shader, vp);
    }
}

void CollectibleManager::checkAllCollisions(const glm::vec3 &playerPosition, float radius) {
    for (auto &collectible : collectibles) {
        if (collectible.checkCollision(playerPosition, radius)) {
            collectible.collect();
            isBoostActive = true;
        }
    }
}

int CollectibleManager::getCollectedCount() const {
    int count = 0;
    for (const auto &collectible : collectibles) {
        if (collectible.isCollected()) {
            ++count;
        }
    }
    return count;
}

int CollectibleManager::getTotalCount() const {
    return collectibleCount;
}

void Collectible::updateLight(Shader &shader, int lightIndex) const {
    if (!collected) {
        std::string lightPos = "pointLights[" + std::to_string(lightIndex) + "].position";
        std::string lightColor = "pointLights[" + std::to_string(lightIndex) + "].color";
        std::string lightIntensity = "pointLights[" + std::to_string(lightIndex) + "].intensity";

        // Pass the collectible light data to the shader
        shader.setVec3(lightPos, position);                      // Light position
        shader.setVec3(lightColor, glm::vec3(1.0f, 0.8f, 0.5f)); // Light color (e.g., gold)
        shader.setFloat(lightIntensity, 1000.0f);                // Light intensity
    }
}

void CollectibleManager::clear() {
    collectibles.clear(); // Clear the vector of collectibles
    std::cout << "All collectibles cleared!" << std::endl;
}

void CollectibleManager::uncollectAll() {
    for (auto &collectible : collectibles) {
        collectible.uncollect();
    }
    // TODO: other than uncollect, also re-randomize collectibles position
}
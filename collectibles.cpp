#include "collectibles.h"

Collectible::Collectible(const glm::vec3 &position, const std::string &type)
    : position(position), type(type), collected(false), model(type) {}

void Collectible::render(Shader &shader, const glm::mat4 &vp) {
    if (!collected) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);             // Identity matrix
        modelMatrix = glm::translate(modelMatrix, position); // Apply position transformation

        shader.use();
        shader.setInt("texture_diffuse", 0); // Tell the shader to use texture unit 0

        // Set model matrix for rendering
        shader.setMat4("model", modelMatrix);
        shader.setMat4("viewProjection", vp);

        // Set emissive color and intensity for the collectible
        shader.setVec3("emissiveColor", glm::vec3(1.0f, 0.8f, 0.2f)); // Golden glow color
        shader.setFloat("emissiveIntensity", 1.0f);                   // Increase intensity for better glow effect

        // Draw the model
        model.Draw(shader);
    }
}

bool Collectible::checkCollision(const glm::vec3 &playerPosition, float radius) {
    float distance = glm::length(playerPosition - position);
    return !collected && (distance < radius);
}

void CollectibleManager::addCollectible(const glm::vec3 &position, const std::string &type) {
    collectibles.emplace_back(position, type);
}

void CollectibleManager::renderAll(Shader &shader, const glm::mat4 &vp) {
    for (auto &collectible : collectibles) {
        collectible.render(shader, vp);
    }
}

void CollectibleManager::checkAllCollisions(const glm::vec3 &playerPosition, float radius) {
    for (auto &collectible : collectibles) {
        if (collectible.checkCollision(playerPosition, radius)) {
            soundManager.playSoundEffect("collect");
            collectible.collect();
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
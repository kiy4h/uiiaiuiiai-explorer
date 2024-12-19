#ifndef COLLECTIBLES_H
#define COLLECTIBLES_H

#include "lib/model.h"
#include "lib/sound_manager.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

class Collectible {
public:
    Collectible(const glm::vec3 &position, const std::string &type);

    void render(Shader &shader, const glm::mat4 &vp);
    void updateLight(Shader &shader, int lightIndex) const;
    bool checkCollision(const glm::vec3 &playerPosition, float radius);

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getLightColor() const { return lightColor; }
    bool isCollected() const { return collected; }
    void collect() {
        // cout << "Collected " << type << " at " << glm::to_string(position) << endl;
        collected = true;
    }
    void uncollect() { collected = false; }

private:
    glm::vec3 position;   // Position of the collectible
    std::string type;     // Type of collectible (e.g., "coin", "gem")
    glm::vec3 lightColor; // Color of the emitted light
    float lightIntensity; // Intensity of the light
    bool collected;       // Has the collectible been collected?
    Model model;          // The model for rendering
};

class CollectibleManager {
public:
    bool isBoostActive = false;
    CollectibleManager(SoundManager &soundManager)
        : soundManager(soundManager) {}
    void addCollectible(const glm::vec3 &position, const std::string &type);
    void uncollectAll();
    void renderAll(Shader &shader, const glm::mat4 &vp);
    void checkAllCollisions(const glm::vec3 &playerPosition, float radius);
    int getCollectedCount() const;
    void setCollectibles(int count) { collectibleCount = count; }
    int getTotalCount() const;
    void clear(); // Clear all collectibles
    std::vector<Collectible> &getCollectibles() { return collectibles; }

private:
    std::vector<Collectible> collectibles;
    int collectibleCount = 0;
    SoundManager &soundManager;
};

#endif // COLLECTIBLES_H

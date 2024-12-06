#ifndef TERRAIN_H
#define TERRAIN_H

#include "model.h"
#include "shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h> // For loading PNG heightmap images
#include <string>
#include <unordered_map>
#include <vector>

class Terrain {
public:
    Terrain(const std::string &heightmapPath, float scale, int width, int height);
    ~Terrain();

    void generateTerrain();
    bool loadTexture(const std::string &texturePath);
    void render(Shader &shader, const glm::mat4 &vp); // Updated to pass the view-projection matrix
    float getHeightAt(float x, float z) const;
    int getWidth() const { return terrainWidth; }
    int getHeight() const { return terrainHeight; }
    // New method: get bounding box corners
    glm::vec3 getMinCorner() const { return minCorner; }
    glm::vec3 getMaxCorner() const { return maxCorner; }

    void addObject(const glm::vec3 &position, const std::string &type);
    void addModel(const std::string &type, const std::string &modelPath); // Load models
    void generateObjects(int count, const std::string &type, float minHeight, float maxHeight, float spread);
    void renderObjects(Shader &objectShader, const glm::mat4 &vp);

private:
    bool loadHeightmap(const std::string &path);
    void setupTerrainBuffers();

    // For culling
    bool isInsideFrustum(const glm::vec4 planes[6]) const; // New method

    unsigned char *heightmapData;
    int heightmapWidth, heightmapHeight;

    int terrainWidth, terrainHeight;
    float terrainScale;

    GLuint terrainVAO, terrainVBO, terrainEBO;
    GLuint textureID;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> texCoords;

    glm::vec3 minCorner; // Minimum point of the terrain AABB
    glm::vec3 maxCorner; // Maximum point of the terrain AABB

    struct ObjectInstance {
        glm::vec3 position;
        int modelIndex; // Index into the model list for the type
        std::string type;
    };
    std::vector<ObjectInstance> objects;                        // Store all placed objects
    std::unordered_map<std::string, std::vector<Model>> models; // Store models for each type
};

#endif // TERRAIN_H

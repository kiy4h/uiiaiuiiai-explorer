#ifndef TERRAIN_H
#define TERRAIN_H

#include "shader.h"
#include <glad/glad.h> // For OpenGL extensions
#include <glm/glm.hpp>
#include <stb/stb_image.h> // For loading PNG heightmap images
#include <string>
#include <vector>

class Terrain {
public:
    Terrain(const std::string &heightmapPath, float scale, int width, int height);
    ~Terrain();

    void generateTerrain();
    bool loadTexture(const std::string &texturePath); // Corrected declaration
    void render(Shader &shader);
    float getHeightAt(float x, float z) const;

    int getWidth() const { return terrainWidth; }
    int getHeight() const { return terrainHeight; }

private:
    bool loadHeightmap(const std::string &path);
    void setupTerrainBuffers();

    unsigned char *heightmapData;
    int heightmapWidth, heightmapHeight;

    int terrainWidth, terrainHeight;
    float terrainScale;

    GLuint terrainVAO, terrainVBO, terrainEBO;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> texCoords;

    // Add this to manage the texture
    GLuint textureID;
};

#endif // TERRAIN_H

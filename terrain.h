#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h> // For OpenGL extensions
#include <glm/glm.hpp>
#include <stb_image.h> // For loading PNG heightmap images
#include <string>
#include <vector>

class Terrain {
public:
    // Constructor with heightmap path, scale factor, and terrain size
    Terrain(const std::string &heightmapPath, float scale, int width, int height);

    // Destructor
    ~Terrain();

    // Method to generate the terrain mesh
    void generateTerrain();

    // Method to render the terrain
    void render();

    // Method to get the height of the terrain at (x, z)
    float getHeightAt(int x, int z) const;

    // Get terrain width
    int getWidth() const { return terrainWidth; }

    // Get terrain height
    int getHeight() const { return terrainHeight; }

private:
    // Helper methods
    bool loadHeightmap(const std::string &path);
    void setupTerrainBuffers();

    // Heightmap data and dimensions
    unsigned char *heightmapData;
    int heightmapWidth, heightmapHeight;

    // Terrain size
    int terrainWidth, terrainHeight;
    float terrainScale;

    // OpenGL buffers for terrain rendering
    GLuint terrainVAO, terrainVBO, terrainEBO;

    // Vertices and indices for terrain mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

#endif // TERRAIN_H

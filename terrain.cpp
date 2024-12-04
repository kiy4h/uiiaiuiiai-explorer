#include "terrain.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h> // Make sure to include OpenGL context libraries
#include <iostream>

// Custom clamp function for versions of C++ before C++17
template <typename T>
T clamp(const T &value, const T &low, const T &high) {
    return (value < low) ? low : (value > high) ? high
                                                : value;
}

Terrain::Terrain(const std::string &heightmapPath, float scale, int width, int height)
    : terrainScale(scale), terrainWidth(width), terrainHeight(height) {
    if (!loadHeightmap(heightmapPath)) {
        std::cerr << "Error loading heightmap!" << std::endl;
    }
    generateTerrain();
    setupTerrainBuffers();
}

Terrain::~Terrain() {
    // Clean up heightmap data and OpenGL buffers
    stbi_image_free(heightmapData);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainEBO);
    glDeleteVertexArrays(1, &terrainVAO);
}

bool Terrain::loadHeightmap(const std::string &path) {
    heightmapData = stbi_load(path.c_str(), &heightmapWidth, &heightmapHeight, nullptr, 1); // Load as grayscale (1 channel)

    if (!heightmapData) {
        std::cerr << "Failed to load heightmap: " << path << std::endl;
        return false; // Indicate failure if heightmap is not loaded
    }

    std::cout << "Heightmap loaded successfully! Width: " << heightmapWidth << ", Height: " << heightmapHeight << std::endl;
    return true;
}

void Terrain::generateTerrain() {
    for (int z = 0; z < terrainHeight; ++z) {
        for (int x = 0; x < terrainWidth; ++x) {
            // Get the height of the terrain at (x, z) using the heightmap
            float height = getHeightAt(x, z);

            // Add vertex (x, y, z) where y is the height
            vertices.push_back(x);      // X
            vertices.push_back(height); // Y (height)
            vertices.push_back(z);      // Z
        }
    }

    // Generate indices for terrain mesh (triangle grid)
    for (int z = 0; z < terrainHeight - 1; ++z) {
        for (int x = 0; x < terrainWidth - 1; ++x) {
            int topLeft = z * terrainWidth + x;
            int bottomLeft = (z + 1) * terrainWidth + x;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topLeft + 1);

            indices.push_back(bottomLeft);
            indices.push_back(bottomLeft + 1);
            indices.push_back(topLeft + 1);
        }
    }
}

void Terrain::setupTerrainBuffers() {
    // Create OpenGL buffers
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Terrain::render() {
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

float Terrain::getHeightAt(int x, int z) const {
    if (!heightmapData) {
        std::cerr << "Error: Heightmap not loaded correctly!" << std::endl;
        return 0.0f; // Return a default value
    }

    // Clamp x and z to the heightmap bounds
    x = clamp(x, 0, heightmapWidth - 1);
    z = clamp(z, 0, heightmapHeight - 1);

    // Debug output for heightmap values
    // std::cout << "Heightmap value at (" << x << ", " << z << "): "
    //           << (int)heightmapData[z * heightmapWidth + x] << std::endl;

    // Return the height by reading the pixel value and scaling it
    return terrainScale * (heightmapData[z * heightmapWidth + x] / 255.0f);
}

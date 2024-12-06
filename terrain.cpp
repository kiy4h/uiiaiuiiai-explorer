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

// Constructor
Terrain::Terrain(const std::string &heightmapPath, float scale, int width, int height)
    : terrainScale(scale), terrainWidth(width), terrainHeight(height), textureID(0) { // Initialize textureID
    if (!loadHeightmap(heightmapPath)) {
        std::cerr << "Error loading heightmap!" << std::endl;
    }
    generateTerrain();
    setupTerrainBuffers();
}

// Destructor
Terrain::~Terrain() {
    stbi_image_free(heightmapData);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainEBO);
    glDeleteVertexArrays(1, &terrainVAO);

    if (textureID != 0) {
        glDeleteTextures(1, &textureID); // Clean up texture
    }
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

            // Scale texture coordinates to make the texture smaller (repeat it more times)
            float u = (float)x / (terrainWidth - 1) * 10.0f;  // Repeat the texture 4 times in X direction
            float v = (float)z / (terrainHeight - 1) * 10.0f; // Repeat the texture 4 times in Z direction
            texCoords.push_back(u);                           // U
            texCoords.push_back(v);                           // V
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
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, (vertices.size() + texCoords.size()) * sizeof(float), nullptr, GL_STATIC_DRAW);

    // Upload the vertices and texture coordinates to the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texCoords.size() * sizeof(float), texCoords.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)(vertices.size() * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Terrain::render(Shader &shader) {
    // Activate the shader
    shader.use();

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("terrainTexture", 0); // Assuming the shader uses a sampler2D called 'terrainTexture'

    // Bind and draw the terrain
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Terrain::loadTexture(const std::string &texturePath) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << texturePath << std::endl;
        std::cerr << "stb_image error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    GLenum format = GL_RGB; // Default format
    if (nrChannels == 4) {
        format = GL_RGBA; // Use RGBA if image has an alpha channel
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture wrapping mode to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap in X direction
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrap in Y direction

    // Optionally, you can also set the min and mag filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return true;
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

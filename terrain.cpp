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
    minCorner = glm::vec3(0.0f, FLT_MAX, 0.0f);                                 // Initialize minCorner
    maxCorner = glm::vec3((float)terrainWidth, -FLT_MAX, (float)terrainHeight); // Initialize maxCorner

    for (int z = 0; z < terrainHeight; ++z) {
        for (int x = 0; x < terrainWidth; ++x) {
            float height = getHeightAt(x, z);

            // Update minCorner and maxCorner
            minCorner.y = glm::min(minCorner.y, height);
            maxCorner.y = glm::max(maxCorner.y, height);
            const float tolerance = 1.0f; // Expand the bounding box slightly
            minCorner -= glm::vec3(tolerance);
            maxCorner += glm::vec3(tolerance);

            vertices.push_back(x);      // X
            vertices.push_back(height); // Y
            vertices.push_back(z);      // Z

            // Scale texture coordinates to make the texture smaller (repeat it more times)
            float u = (float)x / (terrainWidth - 1) * 30.0f;  // Repeat the texture 4 times in X direction
            float v = (float)z / (terrainHeight - 1) * 30.0f; // Repeat the texture 4 times in Z direction
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

bool Terrain::isInsideFrustum(const glm::vec4 planes[6]) const {
    for (int i = 0; i < 6; ++i) {
        bool isOutside = true;

        // Check all 8 corners of the bounding box against the plane
        for (const auto &corner : {
                 glm::vec4(minCorner.x, minCorner.y, minCorner.z, 1.0f),
                 glm::vec4(maxCorner.x, minCorner.y, minCorner.z, 1.0f),
                 glm::vec4(minCorner.x, maxCorner.y, minCorner.z, 1.0f),
                 glm::vec4(maxCorner.x, maxCorner.y, minCorner.z, 1.0f),
                 glm::vec4(minCorner.x, minCorner.y, maxCorner.z, 1.0f),
                 glm::vec4(maxCorner.x, minCorner.y, maxCorner.z, 1.0f),
                 glm::vec4(minCorner.x, maxCorner.y, maxCorner.z, 1.0f),
                 glm::vec4(maxCorner.x, maxCorner.y, maxCorner.z, 1.0f)}) {
            if (glm::dot(planes[i], corner) >= 0.0f) {
                isOutside = false; // At least one point is inside
                break;
            }
        }

        if (isOutside) {
            return false; // The box is outside this plane
        }
    }
    return true; // The box is inside or intersects all planes
}

void Terrain::render(Shader &shader, const glm::mat4 &vp) {
    // Calculate frustum planes
    glm::vec4 planes[6];
    planes[0] = glm::vec4(vp[3] + vp[0]);
    planes[1] = glm::vec4(vp[3] - vp[0]);
    planes[2] = glm::vec4(vp[3] + vp[1]);
    planes[3] = glm::vec4(vp[3] - vp[1]);
    planes[4] = glm::vec4(vp[3] + vp[2]);
    planes[5] = glm::vec4(vp[3] - vp[2]);
    for (int i = 0; i < 6; ++i) {
        planes[i] = glm::normalize(planes[i]);
    }

    // Perform frustum culling
    if (!isInsideFrustum(planes)) {
        std::cout << "Failed to load texture" << std::endl;
    }

    // Bind the terrain's texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("terrainTexture", 0); // Tell the shader to use texture unit 0

    // Render the terrain
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Terrain::loadTexture(const std::string &texturePath) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    std::cout << "Generated bind ID: " << textureID << std::endl;

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

float Terrain::getHeightAt(float x, float z) const {
    // Convert the coordinates to integers for the four corners
    int x0 = static_cast<int>(x);
    int z0 = static_cast<int>(z);
    int x1 = std::min(x0 + 1, heightmapWidth - 1);  // Ensure x1 doesn't go out of bounds
    int z1 = std::min(z0 + 1, heightmapHeight - 1); // Ensure z1 doesn't go out of bounds

    // Get the heights of the four surrounding points (top-left, top-right, bottom-left, bottom-right)
    float h00 = heightmapData[z0 * heightmapWidth + x0]; // top-left
    float h10 = heightmapData[z0 * heightmapWidth + x1]; // top-right
    float h01 = heightmapData[z1 * heightmapWidth + x0]; // bottom-left
    float h11 = heightmapData[z1 * heightmapWidth + x1]; // bottom-right

    // Perform bilinear interpolation to calculate the height at (x, z)
    float tx = x - x0; // Relative x-coordinate within the cell
    float tz = z - z0; // Relative z-coordinate within the cell

    // Interpolate along x-direction (top and bottom)
    float h0 = h00 + (h10 - h00) * tx;
    float h1 = h01 + (h11 - h01) * tx;

    // Interpolate along z-direction (final height)
    float height = h0 + (h1 - h0) * tz;

    // Scale the height as before
    return terrainScale * (height / 255.0f);
}

void Terrain::addModel(const std::string &type, const std::string &modelPath) {
    Model model(modelPath); // Assuming Model is a class for loading and managing 3D models
    models[type].push_back(model);
}

void Terrain::generateObjects(int count, const std::string &type, float minHeight, float maxHeight, float spread) {
    for (int i = 0; i < count; ++i) {
        // Generate random position on the terrain
        float x = static_cast<float>(rand() % terrainWidth);
        float z = static_cast<float>(rand() % terrainHeight);
        float y = getHeightAt(x, z);

        // Only place the object if it falls within the height range
        if (y >= minHeight && y <= maxHeight) {

            // Randomly select a model from the available models for this type
            int modelIndex = rand() % models[type].size();

            // Add the object to the list
            objects.push_back({glm::vec3(x, getHeightAt(x, z), z), modelIndex, type});
        }
        // cout << "Object generated at: " << x << ", " << y << ", " << z << endl;
    }
}

void Terrain::renderObjects(Shader &objectShader, const glm::mat4 &vp) {
    objectShader.use();

    for (const auto &object : objects) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, object.position);

        if (object.type == "tree") {
            model = glm::scale(model, glm::vec3(1.0f)); // Example scaling
        } else if (object.type == "batu") {
            model = glm::scale(model, glm::vec3(0.5f));
        } else if (object.type == "grass") {
            model = glm::scale(model, glm::vec3(0.2f));
        } else if (object.type == "batu1") {
            model = glm::scale(model, glm::vec3(0.5f));
        }

        objectShader.setMat4("model", model);
        objectShader.setMat4("viewProjection", vp);

        // Render the selected model
        models[object.type][object.modelIndex].Draw(objectShader);
    }
}

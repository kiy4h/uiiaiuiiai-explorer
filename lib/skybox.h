#ifndef SKYBOX_H
#define SKYBOX_H

#include "camera.h"
#include "shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Skybox {
public:
    Skybox(const std::vector<std::string> &faces, Shader &skyboxShader);
    ~Skybox();

    void render(const glm::mat4 &view, const glm::mat4 &projection, Camera *camera);

private:
    unsigned int loadCubemap(const std::vector<std::string> &faces);
    void setupSkybox();

    unsigned int skyboxVAO, skyboxVBO, cubemapTexture;
    Shader &skyboxShader;
};

#endif // SKYBOX_H

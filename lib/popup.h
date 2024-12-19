#ifndef POPUP_H
#define POPUP_H

#include "lib/shader.h"
#include "text_renderer.h"
#include <glm/glm.hpp>
#include <string>

class Popup {
private:
    std::string message;       // Popup message
    glm::vec3 textColor;       // Color of the text
    glm::vec4 backgroundColor; // Background overlay color
    bool visible;              // Whether the popup is currently visible

public:
    Popup(const std::string &msg, const glm::vec3 &textCol, const glm::vec4 &bgCol)
        : message(msg), textColor(textCol), backgroundColor(bgCol), visible(false) {}

    void show() {
        visible = true;
    }

    void hide() {
        visible = false;
    }

    bool isVisible() const {
        return visible;
    }

    void render(TextRenderer &textRenderer, Shader &overlayShader, Shader &textShader, float screenWidth, float screenHeight) {
        if (!visible) return;

        // Render background overlay
        overlayShader.use();
        overlayShader.setVec4("overlayColor", backgroundColor);
        glDisable(GL_DEPTH_TEST); // Ensure the overlay renders over everything
        renderFullScreenOverlay(overlayShader);

        // Render popup message
        textShader.use(); // Use the passed-in text shader
        textRenderer.RenderText(
            textShader,
            message,
            screenWidth / 2 - message.length() * 12, // Adjust position for centering
            screenHeight / 2,
            1.5f,
            textColor);
        glEnable(GL_DEPTH_TEST); // Re-enable depth testing
    }

private:
    void renderFullScreenOverlay(Shader &shader) {
        static unsigned int VAO = 0, VBO = 0;
        if (VAO == 0) {
            float vertices[] = {
                // Positions
                -1.0f, 1.0f,  // Top-left
                -1.0f, -1.0f, // Bottom-left
                1.0f, -1.0f,  // Bottom-right
                1.0f, -1.0f,  // Bottom-right
                1.0f, 1.0f,   // Top-right
                -1.0f, 1.0f   // Top-left
            };

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        }

        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
};

#endif

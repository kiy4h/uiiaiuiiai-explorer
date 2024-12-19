#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "terrain.h"
#include <iostream>
#include <vector>

// ����������˶��ļ������ܵ�ѡ��
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Ĭ��ֵ
const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 80.0f;
const float ZOOM_RANGE = 20.0f;
const float ZOOM_SPEED = 2.0f;
const float STADIA = 1000.0f;

// ���������
// ��������ͼ�����Ӧ��ŷ���ǡ������;���������OpenGL
class Camera {
public:
    // ���������
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // ŷ����
    float Yaw;   // ƫ����
    float Pitch; // ��б��
    // �����ѡ��
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    // ���캯��
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
    }

    // ���캯���ͱ���ֵ
    Camera(
        float posX, float posY, float posZ,
        float upX, float upY, float upZ,
        float yaw,
        float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
    }

    // ������ͼ����
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // ����ͶӰ����
    glm::mat4 GetProjMatrix(float aspect) {
        return glm::perspective(glm::radians(Zoom), aspect, 0.1f, STADIA);
    }

    // ���ܼ�������
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw -= xoffset;
        Pitch -= yoffset;

        // Constrain the pitch to prevent flipping
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    void ProcessMouseScroll(float yoffset) {
        if (Zoom >= 1.0f && Zoom <= ZOOM)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= ZOOM)
            Zoom = ZOOM;
    }

    void FixView(glm::vec3 position, float yaw) {
        Position = position;
        Yaw = yaw;
        Pitch = 0.0f;
    }

    void ZoomIn() {
        if (Zoom >= ZOOM - ZOOM_RANGE)
            Zoom -= ZOOM_SPEED;
    }

    void ZoomOut() {
        if (Zoom <= ZOOM + ZOOM_RANGE)
            Zoom += ZOOM_SPEED;
    }

    void ZoomRecover() {
        if (Zoom < ZOOM)
            Zoom += ZOOM_SPEED / 2;
        if (Zoom > ZOOM)
            Zoom -= ZOOM_SPEED / 2;
    }
    void updateCameraVectors(glm::vec3 modelPosition, float distance, Terrain *terrain, float dynamicYOffset, bool applyYOffset) {
        // Calculate the offset using spherical coordinates
        glm::vec3 offset;
        offset.x = distance * cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
        offset.y = distance * sin(glm::radians(Pitch));
        offset.z = distance * cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));

        // Calculate the desired camera position
        glm::vec3 desiredPosition = modelPosition + offset;

        // Ensure the camera doesn't go below the terrain
        float terrainHeight = terrain->getHeightAt(desiredPosition.x, desiredPosition.z);
        desiredPosition.y = glm::max(desiredPosition.y, terrainHeight + 2.0f); // Ensure camera stays above terrain

        // Update the camera position
        Position = desiredPosition;

        // Calculate the target position
        glm::vec3 targetPosition = modelPosition;

        // Apply the dynamic yOffset only if specified
        if (applyYOffset) {
            float maxTargetHeight = terrainHeight + dynamicYOffset;
            targetPosition.y = glm::max(targetPosition.y, maxTargetHeight);
        }

        // Ensure the camera points at the target
        Front = glm::normalize(targetPosition - Position);

        // Calculate the Right vector (perpendicular to Front and WorldUp)
        Right = glm::normalize(glm::cross(Front, WorldUp));

        // Recalculate the Up vector (perpendicular to Right and Front)
        Up = glm::normalize(glm::cross(Right, Front));
    }

private:
};

#endif
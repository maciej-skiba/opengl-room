#pragma once

#include "common/gl_includes.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <core/Window.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float defaultYaw         = -45.0f;
const float defaultPitch       =  0.0f;
const float defaultSpeed       =  4.0f;
const float defaultSensitivity =  1.0f;
const float defaultZoom        =  45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float CameraSpeed;
    float MouseSensitivity;
    float Zoom;
    float targetYaw, targetPitch;
    float mouseSmoothingStrength = 16.0f;
    glm::vec3 targetPosition;
    float moveSmoothing = 0.1f;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = defaultYaw, float pitch = defaultPitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), CameraSpeed(defaultSpeed), MouseSensitivity(defaultSensitivity), Zoom(defaultZoom)
    {
        Position = position;
        targetPosition = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        targetYaw = yaw;
        targetPitch = pitch;
        updateCameraVectors();
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), CameraSpeed(defaultSpeed), MouseSensitivity(defaultSensitivity), Zoom(defaultZoom)
    {
        Position = glm::vec3(posX, posY, posZ);
        targetPosition = Position;
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        targetYaw = yaw;
        targetPitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement direction)
    {
        float velocity = CameraSpeed * Window::deltaTime;

        switch (direction)
        {
            case FORWARD:
                targetPosition += Front * velocity;
                break;

            case BACKWARD:
                targetPosition -= Front * velocity;
                break;

            case LEFT:
                targetPosition -= Right * velocity;
                break;

            case RIGHT:
                targetPosition += Right * velocity;
                break;

            case UP:
                targetPosition += Up * velocity;
                break;

            case DOWN:
                targetPosition -= Up * velocity;
                break;

            default:
                break;
        }
    }

    void ProcessKeyboardWithWallColliderSimulation(Camera_Movement direction)
    {
        float velocity = CameraSpeed * Window::deltaTime;
        const float roomHalfSize = 5.0f;
        const float roomHeight   = 3.0f;
        const float margin       = 0.3f; // zapobiega clippingowi w ścianach

        glm::vec3 proposed = targetPosition;

        switch (direction)
        {
            case FORWARD:
            {
                glm::vec3 next = targetPosition + Front * velocity;
                if (next.x < roomHalfSize - margin && next.x > -roomHalfSize + margin &&
                    next.z < roomHalfSize - margin && next.z > -roomHalfSize + margin)
                {
                    proposed = next;
                }
            } break;

            case BACKWARD:
            {
                glm::vec3 next = targetPosition - Front * velocity;
                if (next.x < roomHalfSize - margin && next.x > -roomHalfSize + margin &&
                    next.z < roomHalfSize - margin && next.z > -roomHalfSize + margin)
                {
                    proposed = next;
                }
            } break;

            case LEFT:
            {
                glm::vec3 next = targetPosition - Right * velocity;
                if (next.x < roomHalfSize - margin && next.x > -roomHalfSize + margin &&
                    next.z < roomHalfSize - margin && next.z > -roomHalfSize + margin)
                {
                    proposed = next;
                }
            } break;

            case RIGHT:
            {
                glm::vec3 next = targetPosition + Right * velocity;
                if (next.x < roomHalfSize - margin && next.x > -roomHalfSize + margin &&
                    next.z < roomHalfSize - margin && next.z > -roomHalfSize + margin)
                {
                    proposed = next;
                }
            } break;

            case UP:
            {
                glm::vec3 next = targetPosition + Up * velocity;
                if (next.y < roomHeight - margin)
                {
                    proposed = next;
                }
            } break;

            case DOWN:
            {
                glm::vec3 next = targetPosition - Up * velocity;
                if (next.y > margin)
                {
                    proposed = next;
                }
            } break;

            default:
                break;
        }

        targetPosition = proposed;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        targetYaw   = Yaw + xoffset;
        targetPitch = Pitch + yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (targetPitch > 89.0f)
                targetPitch = 89.0f;
            if (targetPitch < -89.0f)
                targetPitch = -89.0f;
        }

        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        float mouseSmoothing = 1.0f - exp(Window::deltaTime * (-mouseSmoothingStrength));
        Yaw = glm::mix(Yaw, targetYaw, mouseSmoothing);
        Pitch = glm::mix(Pitch, targetPitch, mouseSmoothing);
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

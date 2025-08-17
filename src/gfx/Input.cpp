#include "common/gl_includes.hpp"
#include "gfx/Input.hpp"
#include "core/Window.hpp"

static bool fPressedLastFrame = false;
bool flashlightOn = false; // definicja globalnej

void ProcessInput(GLFWwindow* window, Camera* camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboardWithWallColliderSimulation(FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboardWithWallColliderSimulation(BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboardWithWallColliderSimulation(LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboardWithWallColliderSimulation(RIGHT);    
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera->ProcessKeyboardWithWallColliderSimulation(DOWN);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)     camera->ProcessKeyboardWithWallColliderSimulation(UP);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fPressedLastFrame) { flashlightOn = !flashlightOn; fPressedLastFrame = true; }
    } else {
        fPressedLastFrame = false;
    }
    camera->Position = glm::mix(camera->Position, camera->targetPosition, camera->moveSmoothing);
}
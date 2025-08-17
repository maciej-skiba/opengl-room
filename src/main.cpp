#include "common/gl_includes.hpp"
#include <iostream>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vertices.hpp" 
#include "camera.hpp"

#include "app/Config.hpp"
#include "core/Window.hpp"
#include "core/InputCallbacks.hpp"
#include "gfx/Input.hpp"
#include "gfx/MeshUtils.hpp"
#include "gfx/Model.hpp"
#include "gfx/Attenuation.hpp"
#include "io/FileLoader.hpp"
#include "Shader.hpp"

const glm::mat4 identityMatrix = glm::mat4(1.0f);
extern bool flashlightOn;

int main(void)
{    GLFWwindow* window;
    int initSuccess = 1;

    if (Window::InitializeOpenGL(window) != initSuccess)
    {
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    unsigned int boxVAO, lightVAO;
    
    const char* roomVertexShaderPath = "../shaders/room_vert.glsl";
    const char* roomFragmentShaderPath = "../shaders/room_frag.glsl";

    const char* lightVertexShaderPath = "../shaders/3.3.light_vert.shad";
    const char* lightFragmentShaderPath = "../shaders/3.3.light_frag.shad";

    Shader roomShader(roomVertexShaderPath, roomFragmentShaderPath);
    Shader lightShader(lightVertexShaderPath, lightFragmentShaderPath);
        
    const char* roomModelPath = "../assets/models/room/room.obj";
    Model ourModel(roomModelPath);

    int numOfVerticesInBox = 36;
    int amountOfLightPoints = 2;

    glm::vec3 dirLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 spotLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 lightPointPositions[] = {
        glm::vec3(-3.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 1.0f, 0.0f),
    };

    glm::vec3 lightPointColors[] = {
        glm::vec3(1, 0.275, 0.855), //pink
        glm::vec3(0.271, 0.808, 1)  //cyan
    };
    
    glm::vec3 dirLightPosition = glm::vec3(20.0f, 20.0f, 0.0f);

    int lightBufferSize = numOfVerticesInBox * 6;
    CreateLightVao(lightVAO, lightBoxVertices, lightBufferSize);

    glEnable(GL_DEPTH_TEST);

    std::unique_ptr<Camera> mainCamera = std::make_unique<Camera>(
        glm::vec3(-4.0f, 1.4f,  4.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSetWindowUserPointer(window, mainCamera.get());

    float lastFrame = 0.0f;
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    float nearClippingPlane = 0.1f;
    float farClippingPlane = 100.0f;

    glm::mat4 boxModelMatrix = identityMatrix;
    glm::mat4 lightModelMatrix = identityMatrix;
    
    while (!glfwWindowShouldClose(window))
    {
        Window::UpdateDeltaTime();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(lightVAO);

        glm::mat4 projectionMatrix = 
            glm::perspective(
                glm::radians(mainCamera->Zoom),
                aspectRatio, 
                nearClippingPlane,
                farClippingPlane);
        
        lightShader.UseProgram();
        
        for (int lightPoint = 0; lightPoint < amountOfLightPoints; lightPoint++)
        {
            lightModelMatrix = glm::translate(identityMatrix, lightPointPositions[lightPoint]);
            lightModelMatrix = glm::scale(lightModelMatrix, glm::vec3(0.5f));
            lightShader.SetUniformMat4("model", lightModelMatrix);
            lightShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
            lightShader.SetUniformMat4("projection", projectionMatrix);
            lightShader.SetUniformVec3("lightColor", lightPointColors[lightPoint]);

            glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        }
        
        roomShader.UseProgram();

        for (size_t i = 0; i < attenuationData.size(); i++) 
        {
            std::string base = "attenuations[" + std::to_string(i) + "].";

            glUniform1f(glGetUniformLocation(roomShader.ID, (base + "minDist").c_str()), attenuationData[i].minDist);
            roomShader.SetUniformFloat(base + "minDist", attenuationData[i].minDist);
            roomShader.SetUniformFloat(base + "maxDist", attenuationData[i].maxDist);
            roomShader.SetUniformFloat(base + "constant", attenuationData[i].constant);
            roomShader.SetUniformFloat(base + "linear", attenuationData[i].linear);
            roomShader.SetUniformFloat(base + "quadratic", attenuationData[i].quadratic);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        roomShader.SetUniformMat4("model", model);
        roomShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
        roomShader.SetUniformMat4("projection", projectionMatrix);
        roomShader.SetUniformVec3("cameraPos", mainCamera->Position);

        roomShader.SetUniformVec3("dirLight[0].position", dirLightPosition);
        roomShader.SetUniformVec3("dirLight[0].ambient", dirLightColor * 0.1f);
        roomShader.SetUniformVec3("dirLight[0].diffuse", dirLightColor);
        roomShader.SetUniformVec3("dirLight[0].specular", dirLightColor);
        roomShader.SetUniformFloat("dirLight[0].lightStrength", 1.0f);

        roomShader.SetUniformVec3("pointLight[0].position", lightPointPositions[0]);
        roomShader.SetUniformVec3("pointLight[0].ambient", lightPointColors[0] * 0.1f);
        roomShader.SetUniformVec3("pointLight[0].diffuse", lightPointColors[0]);
        roomShader.SetUniformVec3("pointLight[0].specular", lightPointColors[0]);
        roomShader.SetUniformFloat("pointLight[0].lightStrength", 4.0f);

        roomShader.SetUniformVec3("pointLight[1].position", lightPointPositions[1]);
        roomShader.SetUniformVec3("pointLight[1].ambient", lightPointColors[1] * 0.1f);
        roomShader.SetUniformVec3("pointLight[1].diffuse", lightPointColors[1]);
        roomShader.SetUniformVec3("pointLight[1].specular", lightPointColors[1]);
        roomShader.SetUniformFloat("pointLight[1].lightStrength", 4.0f);
        
        roomShader.SetUniformVec3("spotLight[0].position", mainCamera->Position);
        roomShader.SetUniformVec3("spotLight[0].direction", mainCamera->Front);
        roomShader.SetUniformFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
        roomShader.SetUniformFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(12.0f)));
        roomShader.SetUniformBool("spotLight[0].on", flashlightOn);
        roomShader.SetUniformVec3("spotLight[0].ambient", spotLightColor * 0.1f);
        roomShader.SetUniformVec3("spotLight[0].diffuse", spotLightColor);
        roomShader.SetUniformVec3("spotLight[0].specular", spotLightColor);
        roomShader.SetUniformFloat("spotLight[0].lightStrength", 1.0f);


        ourModel.Draw(roomShader);

        glfwSwapBuffers(window);
        ProcessInput(window, mainCamera.get());
        mainCamera->updateCameraVectors();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
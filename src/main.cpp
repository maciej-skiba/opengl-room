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
#include "gfx/Gui.hpp"
#include "io/FileLoader.hpp"
#include "Shader.hpp"

const glm::mat4 identityMatrix = glm::mat4(1.0f);
extern bool flashlightOn;

int main(void)
{    
    GLFWwindow* window;
    int initSuccess = 1;

    if (Window::InitializeOpenGL(window) != initSuccess)
    {
        return -1;
    }

    Gui::ImGuiInit(window);

    stbi_set_flip_vertically_on_load(true);

    unsigned int boxVAO, lightBoxVao;
    
    const char* roomVertexShaderPath = "../shaders/room.vert";
    const char* roomFragmentShaderPath = "../shaders/room.frag";

    const char* lightBoxVertexShaderPath = "../shaders/lightBox.vert";
    const char* lightBoxFragmentShaderPath = "../shaders/lightBox.frag";

    const char* screenVertexShaderPath = "../shaders/screen.vert";
    const char* screenFragmentShaderPath = "../shaders/screen.frag";

    const char* blurVertexShaderPath = "../shaders/blur.vert";
    const char* blurFragmentShaderPath = "../shaders/blur.frag";

    Shader roomShader(roomVertexShaderPath, roomFragmentShaderPath);
    Shader lightBoxShader(lightBoxVertexShaderPath, lightBoxFragmentShaderPath);
    Shader screenShader(screenVertexShaderPath, screenFragmentShaderPath);
    Shader blurShader(blurVertexShaderPath, blurFragmentShaderPath);
        
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
        glm::vec3(2.5f, 0.550f, 2.15f),
        glm::vec3(0.7f, 2.0f, 2.5f)
    };
    
    int lightBufferSize = numOfVerticesInBox * 6;
    CreateLightVao(lightBoxVao, lightBoxVertices, lightBufferSize);

    std::unique_ptr<Camera> mainCamera = std::make_unique<Camera>(
        glm::vec3(-4.0f, 1.4f,  4.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSetWindowUserPointer(window, mainCamera.get());

    float lastFrame = 0.0f;
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    float nearClippingPlane = 0.1f;
    float farClippingPlane = 100.0f;

    glm::mat4 lightModelMatrix = identityMatrix;
    glm::mat4 projectionMatrix = 
        glm::perspective(
            glm::radians(mainCamera->Zoom),
            aspectRatio, 
            nearClippingPlane,
            farClippingPlane);

    roomShader.UseProgram();

    roomShader.SetUniformFloat("attenuation.constant", attenuationData[0].constant);
    roomShader.SetUniformFloat("attenuation.linear", attenuationData[0].linear);
    roomShader.SetUniformFloat("attenuation.quadratic", attenuationData[0].quadratic);
    roomShader.SetUniformMat4("projection", projectionMatrix);

    roomShader.SetUniformVec3("pointLight[0].position", lightPointPositions[0]);
    roomShader.SetUniformVec3("pointLight[0].ambient", lightPointColors[0] * 0.1f);
    roomShader.SetUniformVec3("pointLight[0].diffuse", lightPointColors[0]);
    roomShader.SetUniformVec3("pointLight[0].specular", lightPointColors[0]);
    roomShader.SetUniformFloat("pointLight[0].lightStrength", 0.6f);

    roomShader.SetUniformVec3("pointLight[1].position", lightPointPositions[1]);
    roomShader.SetUniformVec3("pointLight[1].ambient", lightPointColors[1] * 0.1f);
    roomShader.SetUniformVec3("pointLight[1].diffuse", lightPointColors[1]);
    roomShader.SetUniformVec3("pointLight[1].specular", lightPointColors[1]);
    roomShader.SetUniformFloat("pointLight[1].lightStrength", 0.6f);
    
    roomShader.SetUniformFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
    roomShader.SetUniformFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(12.0f)));
    roomShader.SetUniformVec3("spotLight[0].ambient", spotLightColor * 0.1f);
    roomShader.SetUniformVec3("spotLight[0].diffuse", spotLightColor);
    roomShader.SetUniformVec3("spotLight[0].specular", spotLightColor);
    roomShader.SetUniformFloat("spotLight[0].lightStrength", 1.0f);

    lightBoxShader.UseProgram();

    lightBoxShader.SetUniformMat4("projection", projectionMatrix);


    // framebuffer configuration
    // -------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create a color attachment texture
    unsigned int textureColorbuffers[2];
    glGenTextures(2, textureColorbuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textureColorbuffers[i]);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA16F,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0,
            GL_RGBA,
            GL_FLOAT,
            nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Important for blur
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            textureColorbuffers[i],
            0
        );
    }

    // make 2 color attachments for FBO so both layouts (location 0 and 1) 
    // in fragment shader go to do the framebuffer
    unsigned int attachments[2] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1
    };

    glDrawBuffers(2, attachments);        

    // create Render Buffer Object to handle stencil and depth buffers in framebuffer
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // =====================================================
    // Ping-pong FBOs for gaussian blur
    // =====================================================

    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);

        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA16F,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0,
            GL_RGBA,
            GL_FLOAT,
            nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            pingpongColorbuffers[i],
            0
        );

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Ping-pong FBO " << i << " is not complete!\n";
        }
    }

    screenShader.UseProgram();
    screenShader.SetUniformInt("sceneTexture", 0);
    screenShader.SetUniformInt("bloomTexture", 1);

    blurShader.UseProgram();
    blurShader.SetUniformInt("image", 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    while (!glfwWindowShouldClose(window))
    {
        // =====================================================
        // 1. Draw entire scene to hdrFBO
        // =====================================================

        Window::UpdateDeltaTime();
        ProcessInput(window, mainCamera.get());

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(lightBoxVao);
        
        lightBoxShader.UseProgram();
        lightBoxShader.SetUniformFloat("emissionStrength", 3.0f);
        
        for (int lightPoint = 0; lightPoint < amountOfLightPoints; lightPoint++)
        {
            lightModelMatrix = glm::translate(identityMatrix, lightPointPositions[lightPoint]);
            lightModelMatrix = glm::scale(lightModelMatrix, glm::vec3(0.5f));
            lightBoxShader.SetUniformMat4("model", lightModelMatrix);
            lightBoxShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
            lightBoxShader.SetUniformVec3("lightColor", lightPointColors[lightPoint]);

            glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        }
        
        roomShader.UseProgram();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        roomShader.SetUniformMat4("model", model);
        roomShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
        roomShader.SetUniformVec3("cameraPos", mainCamera->Position);
        roomShader.SetUniformVec3("spotLight[0].position", mainCamera->Position);
        roomShader.SetUniformVec3("spotLight[0].direction", mainCamera->Front);
        roomShader.SetUniformBool("spotLight[0].on", flashlightOn);

        ourModel.Draw(roomShader);

        // =====================================================
        // 2. Blur bright texture using ping-pong FBOs
        // Input: colorBuffers[1]
        // Output: pingpongColorbuffers[!horizontal]
        // =====================================================

        bool horizontal = true;
        bool firstIteration = true;
        constexpr int blurAmount = 10;

        glDisable(GL_DEPTH_TEST);

        blurShader.UseProgram();

        for (int i = 0; i < blurAmount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

            blurShader.SetUniformBool("horizontal", horizontal);

            glActiveTexture(GL_TEXTURE0);

            if (firstIteration)
            {
                glBindTexture(GL_TEXTURE_2D, textureColorbuffers[1]);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
            }

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            horizontal = !horizontal;

            if (firstIteration)
            {
                firstIteration = false;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // ======================
        // 3. Draw quad using default framebuffer
        // (scene + bloom)
        // ======================

        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.UseProgram();
        screenShader.SetUniformBool("bloom", true);
        screenShader.SetUniformFloat("bloomStrength", 1.0f);
        screenShader.SetUniformFloat("exposure", 1.0f);
        screenShader.SetUniformFloat("gamma", 2.2f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffers[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        Gui::ImGuiFrame(window);
        glfwSwapBuffers(window);
        mainCamera->updateCameraVectors();
        glfwPollEvents();
    }

    glfwTerminate();
    Gui::ImGuiShutdown();
    return 0;
}
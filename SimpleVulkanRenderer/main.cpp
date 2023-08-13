#include <chrono>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <stdlib.h>
#include <time.h>

#include "VulkanRenderer.hpp"
#include "VulkanVertexShader.hpp"
#include "VulkanFragmentShader.hpp"
#include "GreedyMesh.h"

constexpr auto WIDTH = 800;
constexpr auto HEIGHT = 600;

// Load the model.
void loadModel(VulkanRenderer& renderer) {
    srand(time(NULL));
    int*** chunkArray = new int** [8];
    for (int x = 0; x < 8; x++) {
        chunkArray[x] = new int* [8];
        for (int y = 0; y < 8; y++) {
            chunkArray[x][y] = new int[8];
            for (int z = 0; z < 8; z++) {
                chunkArray[x][y][z] = rand() % 2;
            }
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    AlgorithmOutput output = greedyMeshAlgorithm(chunkArray, 8, -1);
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
    std::cout << "Execution Time: " << duration.count() << " ms";
    renderer.vertices.reserve(renderer.vertices.size() + output.verticies.size());
    renderer.vertices.insert(renderer.vertices.end(), output.verticies.begin(), output.verticies.end());

    renderer.indices.reserve(renderer.indices.size() + output.indicies.size());
    renderer.indices.insert(renderer.indices.end(), output.indicies.begin(), output.indicies.end());
}

std::shared_ptr<VulkanVertexShader> CreateVertexShader(VkDevice device)
{
    auto vertexShader = std::make_shared<VulkanVertexShader>(device, "main", "shaders/vert.spv");
    vertexShader->VertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::pos));
    vertexShader->VertexAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::color));
    
    vertexShader->VertexUniformBinding(0, sizeof(Vertex));

    return vertexShader;
}

std::shared_ptr<VulkanFragmentShader> CreateFragmentShader(VkDevice device)
{
    auto fragmentShader = std::make_shared<VulkanFragmentShader>(device, "main", "shaders/frag.spv");

    return fragmentShader;
}

int main() {
    VulkanRenderer renderer;

    renderer.CreateGLFWWindow(WIDTH, HEIGHT, "Test Renderer Application");

    VulkanInstanceInfo instanceInfo;
    instanceInfo.ApplicationName = "Test Application";
    instanceInfo.ApplicationVersion = VK_MAKE_VERSION(1, 0, 0);
    renderer.CreateVulkanInstance(instanceInfo);

    renderer.SetupDebugMessenger();
    renderer.CreateGLFWSurface();
    renderer.SelectPhysicalDevice();
    renderer.CreateLogicalDevice();

    SwapChainDescriptor swapChainDescriptor;
    renderer.SetupSwapChain(swapChainDescriptor);
    renderer.CreateDescriptorSetLayout();

    GraphicsPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.VertexShader = CreateVertexShader(renderer.device);
    pipelineDescriptor.FragmentShader = CreateFragmentShader(renderer.device);
    renderer.CreateGraphicsPipeline(pipelineDescriptor);

    renderer.CreateCommandPool();
    //renderer.CreateTextureImage();
    //renderer.CreateTextureImageView();
    //createTextureSampler();
    loadModel(renderer);
    renderer.CreateVertexBuffer();
    renderer.CreateIndexBuffer();
    renderer.CreateUniformBuffers();
    renderer.CreateDescriptorPool();
    renderer.CreateDescriptorSets();
    renderer.CreateCommandBuffers();
    renderer.CreateSyncObjects();

    // Main Loop::
    renderer.modelMatrix = glm::mat4(1.0f);
    renderer.modelMatrix = glm::scale(renderer.modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.modelMatrix = glm::rotate(renderer.modelMatrix, (float)glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    renderer.modelMatrix = glm::translate(renderer.modelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));

    // Keep track of the last loop time.
    auto lastLoopTime = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count() / 1000000000.0;
    // Event loop.
    while (!glfwWindowShouldClose(renderer.window)) {
        glfwPollEvents();

        auto time = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count() / 1000000000.0;
        // The delta time in seconds.
        float deltaTime = time - lastLoopTime;
        lastLoopTime = time;

        // Display the FPS on the title.
        glfwSetWindowTitle(renderer.window, ("Vulkan Test | FPS: " + std::to_string(1 / deltaTime)).c_str());

        // If the left key is press, rotate the object left.
        int leftKeyState = glfwGetKey(renderer.window, GLFW_KEY_LEFT);
        if (leftKeyState == GLFW_PRESS) {
            renderer.modelMatrix = glm::rotate(renderer.modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        }

        // If the right key is pressed, rotate the object right.
        int rightKeyState = glfwGetKey(renderer.window, GLFW_KEY_RIGHT);
        if (rightKeyState == GLFW_PRESS) {
            renderer.modelMatrix = glm::rotate(renderer.modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // If the right key is pressed, rotate the object right.
        int shiftKeyState = glfwGetKey(renderer.window, GLFW_KEY_LEFT_SHIFT);
        if (shiftKeyState == GLFW_PRESS) {
            renderer.modelMatrix = glm::translate(renderer.modelMatrix, glm::vec3(0, -1 * deltaTime, 0));
        }

        renderer.drawFrame();
    }

    vkDeviceWaitIdle(renderer.device);

    renderer.cleanup();

    return EXIT_SUCCESS;
}
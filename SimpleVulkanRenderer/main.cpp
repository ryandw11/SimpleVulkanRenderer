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

#include <atomic>
#include <thread>

#include "VulkanRenderer.hpp"
#include "VulkanVertexShader.hpp"
#include "VulkanFragmentShader.hpp"
#include "VulkanTexture.hpp"
#include "VulkanMappedBuffer.hpp"
#include "Camera.hpp"
#include "Chunk.hpp"

#include "DemoConsts.hpp"

constexpr auto WIDTH = 1080;
constexpr auto HEIGHT = 720;

std::shared_ptr<VulkanRenderer> renderer;

/*
    =============================
    Scene Data
    =============================
*/
Camera camera;
std::vector<Ptr(Chunk)> chunks;

VulkanBuffer vertexBuffer;
std::vector<Vertex> vertices;

VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
std::vector<uint32_t> indices;

VulkanMappedBuffer modelMatrixBuffer;

VulkanQueue resourceLoadingQueue;

// The struct for uniforms.
struct UniformBufferObject {
    // Explicitly align the data for uniforms
    // mat4 must align on 16 bytes.
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};
VulkanFrameObject<VulkanMappedBuffer> mappedUniformBuffers;
glm::mat4 modelMatrix;

std::vector<Vertex> cubeVertices =
{
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            //back
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            //top
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            //bottom
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            //right
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            // left
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
    };

const std::vector<Vertex> vers = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint32_t> ind = {
    0, 1, 2, 2, 3, 0
};

std::vector<uint32_t> cubeIndices = {
            0, 1, 2, 2, 3, 0,
            // back
            4, 7, 6, 6, 5, 4,
            // top
            8, 9, 10, 10, 11, 8,
            // bottom
            12, 15, 14, 14, 13, 12,
            // right
            16, 17, 18, 18, 19, 16,
            // left
            20, 21, 22, 22, 23, 20
};

// ========================= [ Chunk Demo Settings ] ==================
constexpr auto NUMBER_OF_CHUNKS = 2;

// ========================= [ Multi Threading] ==================

std::atomic_bool finishedLoadingChunk = false;
std::atomic_bool finishedLoadingChunk2 = false;
std::thread chunkLoadingThread;

// Load the model.
/*void loadModel() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Started load thread" << std::endl;
    std::this_thread::sleep_for (std::chrono::seconds(1));
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
    vertices.reserve(vertices.size() + output.verticies.size());
    vertices.insert(vertices.end(), output.verticies.begin(), output.verticies.end());

    indices.reserve(indices.size() + output.indicies.size());
    indices.insert(indices.end(), output.indicies.begin(), output.indicies.end());

    auto pool = renderer->CreateCommandPool("ResourceLoader", resourceLoadingQueue);

    // Vertex Buffer
    vertexBuffer = renderer->mBufferUtilities->CreateVertexBuffer(vertices, pool->CommandPool(), resourceLoadingQueue.queue);

    // Index Buffer
    renderer->mBufferUtilities->CreateIndexBuffer(indices, indexBuffer, indexBufferMemory, pool->CommandPool(), resourceLoadingQueue.queue);

    finishedLoadingChunk = true;
    std::cout << "Finished load thread" << std::endl;
}*/

void PopulateChunks(int sx, int sy, int sz)
{
    for (int x = 0; x < sx; x++)
    {
        for (int y = 0; y < sy; y++)
        {
            for (int z = 0; z < sz; z++)
            {
                chunks.push_back(std::make_shared<Chunk>(glm::vec3( x * CHUNK_VOXEL_COUNT, y * CHUNK_VOXEL_COUNT, z * CHUNK_VOXEL_COUNT)));
            }
        }
    }
}

void LoadChunks()
{
    auto pool = renderer->CreateCommandPool("ResourceLoader", resourceLoadingQueue);

    for (auto& chunk : chunks)
    {
        chunk->GenerateChunk(renderer->mBufferUtilities, pool, resourceLoadingQueue);
    }
}

void StartLoading() {
    chunkLoadingThread = std::thread(LoadChunks);
}

// ========================= [ Multi Threading] ==================

std::shared_ptr<VulkanVertexShader> CreateVertexShader(VkDevice device)
{
    auto vertexShader = std::make_shared<VulkanVertexShader>(device, "main", "shaders/vert.spv");
    vertexShader->VertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::pos));
    vertexShader->VertexAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::color));
    vertexShader->VertexAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, Vertex::texCoord));
    vertexShader->VertexAttributeMatrix4f(1, 3);
    
    vertexShader->VertexUniformBinding(0, sizeof(Vertex));
    vertexShader->VertexUniformBinding(1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE);

    return vertexShader;
}

std::shared_ptr<VulkanFragmentShader> CreateFragmentShader(VkDevice device)
{
    auto fragmentShader = std::make_shared<VulkanFragmentShader>(device, "main", "shaders/frag.spv");

    return fragmentShader;
}

void SetupBuffers()
{
    // Model Matrix Buffer
    renderer->mBufferUtilities->CreateBuffer(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelMatrixBuffer, modelMatrixBuffer);
    renderer->mBufferUtilities->MapMemory(modelMatrixBuffer, 0, sizeof(glm::mat4) * 2, 0, modelMatrixBuffer.DirectMappedMemory());
    // Uniform Buffers
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    mappedUniformBuffers = VulkanFrameObject<VulkanMappedBuffer>(2 /*Swapchain Size*/);
    // Create a uniform buffer for each swapchain image.
    for (size_t i = 0; i < 2; i++) {
        renderer->mBufferUtilities->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mappedUniformBuffers[i], mappedUniformBuffers[i]);
        renderer->mBufferUtilities->MapMemory(mappedUniformBuffers[i], 0, sizeof(UniformBufferObject), 0, mappedUniformBuffers[i].DirectMappedMemory());
    }
}
void UpdateUniformBuffer(uint32_t currentImage) {
    // Use static to keep track of the previous time.
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    // The delta time in seconds.
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    for (int i = 0; i < chunks.size(); i++)
    {
        if (chunks[i]->FinishedGenerating() && chunks[i]->IndiciesSize() > 0)
        {
            glm::mat4 chunkModelMatrix = glm::translate(modelMatrix, chunks[i]->Location());
            //modelMatrices[i] = glm::mat4(1.0f);
            //modelMatrices[i] = modelMatrix;
            memcpy(chunks[i]->ModelBuffer().MappedMemory(), &chunkModelMatrix, sizeof(glm::mat4));
        }
    }

    UniformBufferObject ubo{};
    // Create the model matrix.
    // This rotates the model on the Z-Axis, accounting for the deltaTime.
    ubo.model = modelMatrix;
    //ubo.model = glm::rotate(modelMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // The view transformation. Loot at the geometry at a 45 degree angle.
    //ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = camera.GetViewMatrix();
    // 45 degree field of view for the projective.
    ubo.proj = glm::perspective(glm::radians(45.0f), renderer->mSwapChain->Extent().width / (float)renderer->mSwapChain->Extent().height, 0.1f, 100.0f);

    // Since GLM was desinged for OpenGL (which has its Y coordinate inverted) we need to flip the Y value in the project matrix.
    ubo.proj[1][1] *= -1;

    // Copy the data in the uniform buffer object to the current uniform buffer.
    memcpy(mappedUniformBuffers[currentImage].MappedMemory(), &ubo, sizeof(ubo));
}


void CleanUpBuffers()
{
    vkDestroyBuffer(renderer->mDevice, indexBuffer, nullptr);
    vkFreeMemory(renderer->mDevice, indexBufferMemory, nullptr);

    vertexBuffer.DestoryBuffer(renderer->mDevice);
    modelMatrixBuffer.DestoryBuffer(renderer->mDevice);

    for (int i = 0; i < 2; i++)
    {
        mappedUniformBuffers[i].DestoryBuffer(renderer->mDevice);
    }
}

int main() {
    srand(time(NULL));

    PopulateChunks(20, 2, 20);

    renderer = std::make_shared<VulkanRenderer>();

    VulkanAutoInitSettings autoInitSettings;
    autoInitSettings.InstanceInfo.ApplicationName = "Test Application";
    autoInitSettings.InstanceInfo.ApplicationVersion = VK_MAKE_VERSION(1, 0, 0);
    autoInitSettings.SetupDebug = true;
    autoInitSettings.WindowHeight = HEIGHT;
    autoInitSettings.WindowWidth = WIDTH;
    autoInitSettings.WindowName = "Test Renderer Application";
    
    VulkanQueueDescriptor queueDescriptor;
    queueDescriptor.Type = COMPUTE_QUEUE;
    queueDescriptor.Priority = 0.7f;
    queueDescriptor.Name = "ResourceLoadingQueue";
    autoInitSettings.CustomQueues.push_back(queueDescriptor);

    renderer->AutoInitialize(
        autoInitSettings,
        [](auto descriptorLayout) { /* Create Default Descriptor Layout */
            // Describes the uniforms that are used in the shaders.
            descriptorLayout->UniformBufferBinding(/*Binding*/ 0, /*Count*/ 1, VK_SHADER_STAGE_VERTEX_BIT);
            descriptorLayout->ImageSamplerBinding(1, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        },
        []() { /* Pipeline Creation Stage */
            GraphicsPipelineDescriptor pipeline;
            pipeline.VertexShader = CreateVertexShader(renderer->mDevice);
            pipeline.FragmentShader = CreateFragmentShader(renderer->mDevice);
            return pipeline;
        },
        []() { /* General Loading Stage */
            //loadModel();
            SetupBuffers();

            resourceLoadingQueue = renderer->GetNamedVulkanQueue("ResourceLoadingQueue");

            // Start the Threading
            StartLoading();
        },
        [](auto setBuilder) { /* Create the default descriptor sets for each framebuffer. */
            VulkanTexture texture("textures/texture.jpg", renderer, renderer->mBufferUtilities);
            setBuilder->DescribeBuffer(0, 0, mappedUniformBuffers, sizeof(UniformBufferObject));
            setBuilder->DescribeImageSample(1, 0, texture.ImageView(), texture.Sampler());
        }
    );

    // Main Loop::
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
    modelMatrix = glm::rotate(modelMatrix, (float)glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-5*CHUNK_VOXEL_COUNT, -2 * CHUNK_VOXEL_COUNT, -5 * CHUNK_VOXEL_COUNT));

    glfwSetCursorPosCallback(renderer->mWindow, [](GLFWwindow* window, double x, double y) {
        camera.mouse_callback(window, x, y);
        });
    glfwSetInputMode(renderer->mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Keep track of the last loop time.
    auto lastLoopTime = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count() / 1000000000.0;
    // Event loop.
    while (!glfwWindowShouldClose(renderer->mWindow)) {
        glfwPollEvents();

        auto time = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count() / 1000000000.0;
        // The delta time in seconds.
        float deltaTime = time - lastLoopTime;
        lastLoopTime = time;

        // Display the FPS on the title.
        glfwSetWindowTitle(renderer->mWindow, ("Vulkan Test | FPS: " + std::to_string(1 / deltaTime)).c_str());

        // If the left key is press, rotate the object left.
        int leftKeyState = glfwGetKey(renderer->mWindow, GLFW_KEY_LEFT);
        if (leftKeyState == GLFW_PRESS) {
            //modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            camera.MoveLeft(5 * deltaTime);
        }

        // If the right key is pressed, rotate the object right.
        int rightKeyState = glfwGetKey(renderer->mWindow, GLFW_KEY_RIGHT);
        if (rightKeyState == GLFW_PRESS) {
           //modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            camera.MoveRight(5 * deltaTime);
        }

        if (glfwGetKey(renderer->mWindow, GLFW_KEY_UP) == GLFW_PRESS) {
            camera.MoveForward(5 * deltaTime);
        }

        if (glfwGetKey(renderer->mWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
            camera.MoveBackward(5 * deltaTime);
        }

        // If the right key is pressed, rotate the object right.
        int shiftKeyState = glfwGetKey(renderer->mWindow, GLFW_KEY_LEFT_SHIFT);
        if (shiftKeyState == GLFW_PRESS) {
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -1 * deltaTime, 0));
        }

        auto currentImage = renderer->StartFrameDrawing();

        UpdateUniformBuffer(currentImage);

        // Record Commands
        auto frameCommandBuffer = renderer->GetFrameCommandBuffer();
        frameCommandBuffer->Reset();
        frameCommandBuffer->StartCommandRecording();
        frameCommandBuffer->StartRenderPass(renderer->RenderPass(), renderer->SwapChain()->FrameBuffers()[currentImage], renderer->SwapChain()->Extent(), {164 / 255.0, 236 / 255.0, 252 / 255.0, 1.0});
        frameCommandBuffer->BindPipeline(renderer->PrimaryGraphicsPipeline()->Pipeline());
        frameCommandBuffer->SetViewportScissor(renderer->SwapChain()->Extent());

        int finishedCount = 0;

        for (auto& chunk : chunks)
        {
            if (chunk->FinishedGenerating() && chunk->IndiciesSize() > 0)
            {
                frameCommandBuffer->BindVertexBuffer(chunk->VertexBuffer());
                frameCommandBuffer->BindIndexBuffer(chunk->IndexBuffer());
                frameCommandBuffer->BindVertexBuffer(chunk->ModelBuffer(), 0, 1); // Bind matrix buffer.
                frameCommandBuffer->BindDescriptorSet(renderer->PrimaryGraphicsPipeline()->PipelineLayout(), renderer->DescriptorHandler()->DescriptorSetBuilder()->GetBuiltDescriptorSets()[currentImage]);
                frameCommandBuffer->DrawIndexed(chunk->IndiciesSize());
                
                finishedCount++;
            }
        }

        frameCommandBuffer->EndRenderPass();
        frameCommandBuffer->EndCommandRecording();


        renderer->EndFrameDrawing(currentImage);

        
        if (finishedCount >= chunks.size() && chunkLoadingThread.joinable())
        {
            chunkLoadingThread.join();
        }
    }

    vkDeviceWaitIdle(renderer->mDevice);

    CleanUpBuffers();
    renderer->cleanup();

    return EXIT_SUCCESS;
}
#pragma once

#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "VulkanIncludes.hpp"

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

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

//#include "GreedyMesh.h"
#include "VulkanRendererTypes.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanDescriptorLayout.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanBufferUtilities.hpp"
#include "VulkanPipelineHolderIntf.hpp"

// The amount of frames the system should try to handle at once.
const int MAX_FRAMES_IN_FLIGHT = 2;

// Specifiy the validation layers.
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Specify the device extesions that need to be used.
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// If not debug, disable validation layers.
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Check the 3 basic properties for swap chains.
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// The struct for uniforms.
struct UniformBufferObject {
    // Explicitly align the data for uniforms
    // mat4 must align on 16 bytes.
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class VulkanRenderer : public VulkanPipelineHolderIntf {
public:
    operator VkPhysicalDevice () override
    {
        return physicalDevice;
    }

    operator VkDevice () override
    {
        return device;
    }

    operator VkQueue () override
    {
        return graphicsQueue;
    }

    operator VkCommandPool () override
    {
        return mCommandPools[0]->CommandPool();
    }
public:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    // Native surface.
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // Logical Device:
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkRenderPass renderPass;
    // The layout for shader descriptors.
    //VkDescriptorSetLayout descriptorSetLayout;

    // Manages allocation of Command Buffers.
    std::vector<std::shared_ptr<VulkanCommandPool>> mCommandPools;

    // The vector of semaphroes for image availability (This is for syncronization).
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;

    // To keep track of CPU -> GPU syncronization.
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;

    size_t currentFrame = 0;

    // A flag that triggers when the window is resized.
    bool framebufferResized = false;

    // Buffers for uniforms. (One for each swap chain image since multiple can be in flight).
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;


    /*
        Custom stuff
    */
    std::shared_ptr<VulkanGraphicsPipeline> mGraphicsPipeline;
    std::shared_ptr<VulkanSwapChain> mSwapChain;
    std::shared_ptr<VulkanDescriptorLayout> mDescriptorHandler;
    std::shared_ptr<VulkanBufferUtilities> mBufferUtilities;

    /*

        Transformation Matrix

    */
    glm::mat4 modelMatrix;


    // Load the vkCreateDebugUtilsMessengerEXT method since it is an extension.
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    // Load the vkDestroyDebugUtilsMessengerEXT method since it is an extension.
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

public:
    void CreateGLFWWindow(int width, int height, std::string name);

    void CreateVulkanInstance(VulkanInstanceInfo instanceInfo);

    /*void drawFrame() {
        // Wait for the current frame fence to finish before making the frame.
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        // Aquire an image from the swap chain.
        // UINT64_MAX defines the timeout (in nanoseconds) for an image to become available. This disables that feature.
        // The fourth parameter is for a syncronization object.
        VkResult result = vkAcquireNextImageKHR(device, mSwapChain->SwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        // If the Swap Chain is out of date and needs to be recreated.
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain images!");
        }

        // Update the uniform buffer for the image.
        updateUniformBuffer(imageIndex);

        // Check if the previous frame is using this image.
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark that the image is currently being used by this frame.
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

        // The fence needs to be reset after being waited for.
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        auto currentCommandBuffer = mCommandPools[0]->CommandBuffers()[currentFrame];
        // Submit to the queue. Signal the fence when this is done.
        currentCommandBuffer->Submit(graphicsQueue, imageAvailableSemaphores[currentFrame], renderFinishedSemaphores[currentFrame], inFlightFences[currentFrame]);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { mSwapChain->SwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        // If the swap chain is out of date or suboptimal, then recreate the swap chain.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        // Increase the current frame by 1 and loop back around when needed.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }*/

    /// <summary>
    /// Forward the start drawing command to the SwapChain.
    /// </summary>
    /// <returns>The image being drawn to.</returns>
    uint32_t StartFrameDrawing()
    {
        return mSwapChain->StartFrameDrawing();
    }

    void EndFrameDrawing(uint32_t currentImage)
    {
        mSwapChain->EndFrameDrawing(graphicsQueue, *(mCommandPools[0]->CommandBuffers()[mSwapChain->CurrentFrame()]), presentQueue, framebufferResized, currentImage);
    }

    /**
        Update the Uniform Buffer of the current image.

        @param currentImage The integer to the current image being processed.
    */
    void updateUniformBuffer(uint32_t currentImage) {
        // Use static to keep track of the previous time.
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        // The delta time in seconds.
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        // Create the model matrix.
        // This rotates the model on the Z-Axis, accounting for the deltaTime.
        ubo.model = modelMatrix;
        //ubo.model = glm::rotate(modelMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // The view transformation. Loot at the geometry at a 45 degree angle.
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // 45 degree field of view for the projective.
        ubo.proj = glm::perspective(glm::radians(45.0f), mSwapChain->Extent().width / (float)mSwapChain->Extent().height, 0.1f, 10.0f);

        // Since GLM was desinged for OpenGL (which has its Y coordinate inverted) we need to flip the Y value in the project matrix.
        ubo.proj[1][1] *= -1;

        // Copy the data in the uniform buffer object to the current uniform buffer.
        void* data;
        vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBuffersMemory[currentImage]);

    }

    // Cleanup the swap chain.
    void cleanupSwapChain() {

        vkDestroyImageView(device, mSwapChain->DepthImageView(), nullptr);
        vkDestroyImage(device, mSwapChain->DepthImage(), nullptr);
        vkFreeMemory(device, mSwapChain->DepthImageMemory(), nullptr);

        for (auto framebuffer : mSwapChain->FrameBuffers()) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto commandPool : mCommandPools)
        {
            commandPool->FreeCommandBuffers(device);
        }

        mGraphicsPipeline->CleanupPipeline(device);
        vkDestroyRenderPass(device, renderPass, nullptr);

        // Destroy the image views.
        for (auto imageView : mSwapChain->ImageViews()) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        // Destroy the swap chain.
        vkDestroySwapchainKHR(device, mSwapChain->SwapChain(), nullptr);

        for (size_t i = 0; i < mSwapChain->Images().size(); i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, mDescriptorHandler->BuiltDescriptorPool(), nullptr);
    }

    void cleanup() {

        cleanupSwapChain();

        vkDestroyDescriptorSetLayout(device, mDescriptorHandler->Layout(), nullptr);

            

        // Cleanup the syncronization objects for the frames.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        for (auto commandPool : mCommandPools)
        {
            commandPool->DestroyCommandPool(device);
        }

        // Destroy the device.
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        // Destroy and terminate the window.
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void CreateBufferUtilities()
    {
        mBufferUtilities = std::make_shared<VulkanBufferUtilities>(physicalDevice, device, mCommandPools[0]->CommandPool(), graphicsQueue);
    }

    // Create the uniform buffer for each swapchain image.
    void CreateUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        uniformBuffers.resize(mSwapChain->Images().size());
        uniformBuffersMemory.resize(mSwapChain->Images().size());

        // Create a uniform buffer for each swapchain image.
        for (size_t i = 0; i < mSwapChain->Images().size(); i++) {
            mBufferUtilities->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        }
    }

    // Recreate the swap chain when needed (like on window resize).
    void recreateSwapChain() {
        /*int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        // This code handles minimization. Wait until the program is unminimized.
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        // Wait until all device resources are not in use.
        vkDeviceWaitIdle(device);

        cleanupSwapChain();
        mSwapChain->InitializeSwapChain(window, surface, physicalDevice, device);
        CreateRenderPass();
        mGraphicsPipeline->UpdatePipeline(device, renderPass, mSwapChain->Extent(), mDescriptorHandler->Layout());
        mSwapChain->CreateDepthImage(physicalDevice, device);
        mSwapChain->CreateFrameBuffers(device, renderPass);
        CreateUniformBuffers();
        CreateDefaultRenderCommandBuffers(nullptr, nullptr, 0); // TODO:: Fix
        */
    }

    // Create objects needed for syncronization.
    //void CreateSyncObjects();

    void CreateDefaultRenderCommandBuffers(VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indices) {
        for (size_t i = 0; i < mSwapChain->FrameBuffers().size(); i++) {
            auto commandBuffer = mCommandPools[0]->CreateCommandBuffer(device);
            commandBuffer->StartCommandRecording();
            commandBuffer->StartRenderPass(renderPass, mSwapChain->FrameBuffers()[i], mSwapChain->Extent(), {164/255.0, 236/255.0, 252/255.0, 1.0});
            commandBuffer->BindPipeline(mGraphicsPipeline->Pipeline());
            commandBuffer->BindVertexBuffer(vertexBuffer);
            commandBuffer->BindIndexBuffer(indexBuffer);
            commandBuffer->BindDescriptorSet(mGraphicsPipeline->PipelineLayout(), mDescriptorHandler->DescriptorSetBuilder()->GetBuiltDescriptorSets()[i]);
            //vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline->PipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
            commandBuffer->DrawIndexed(indices);
            commandBuffer->EndRenderPass();
            commandBuffer->EndCommandRecording();
        }
    }

    // Create the command buffers.
    // TODO:: Make command system customizable.
    void CreateCommandPool(std::string identifier) {
        mCommandPools.push_back(std::make_shared<VulkanCommandPool>(surface, physicalDevice, device, identifier));
    }

    void CreateRenderPass();
    void CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor);

    void SetupSwapChain(const SwapChainDescriptor descriptor);

    void CreateGLFWSurface();

    void CreateLogicalDevice();

    // Pick a graphics card to use.
    void SelectPhysicalDevice();

    // This checks to make sure that the Device can handle all of the
    // features that we use.
    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indicies = findQueueFamilies(device);

        bool extensionSupported = checkDeviceExtensionSupport(device);

        // Check is swap chain is adequate.
        bool swapChainAdequate = false;
        if (extensionSupported) {
            VulkanSwapChain::SwapChainSupportDetails swapChainSupport = VulkanSwapChain::QuerySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // Get PhysicalDevice features.
        VkPhysicalDeviceFeatures supportedFeatures{};
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indicies.isComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }


    // Check if extensions are suppored for a sepecific physical device.
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // Remove required extension if it exists.
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        // Extensions are supported if the list is empty.
        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // Find a queue family that supports QUEUE GRAPHICS BIT.

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            // Find one that supports presentation. (Can be different than the graphics queue)
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            // Check if the int is populated.
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void SetupDebugMessenger();

    // Get the list of required extensions.
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    // Read all of the bytes in a file.
    static std::vector<char> readFile(const std::string& filename) {
        // Open the file starting at the end.
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file!");
        }

        // Get the file size by getting the position.
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        // Go to beginning and read all of the data.
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    // A GLFW callback for when the window is resized.
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
};


#endif
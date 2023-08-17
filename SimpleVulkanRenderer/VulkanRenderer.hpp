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
        return mDefaultCommandPool->CommandPool();
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

    // Manages allocation of Command Buffers.
    std::shared_ptr<VulkanCommandPool> mDefaultCommandPool;

    // A flag that triggers when the window is resized.
    bool framebufferResized = false;


    /*
        Custom stuff
    */
    std::shared_ptr<VulkanGraphicsPipeline> mGraphicsPipeline;
    std::shared_ptr<VulkanSwapChain> mSwapChain;
    std::shared_ptr<VulkanDescriptorLayout> mDescriptorHandler;
    std::shared_ptr<VulkanBufferUtilities> mBufferUtilities;

public:
    // ----------------------------------------------------
    // Initalizers
    // ----------------------------------------------------
    void CreateGLFWWindow(int width, int height, std::string name);
    void CreateVulkanInstance(VulkanInstanceInfo instanceInfo);
    void CreateGLFWSurface();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void SetupSwapChain(const SwapChainDescriptor descriptor);
    void CreateRenderPass();
    void CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor);
    void CreateDefaultCommandPool(std::string identifier) {
        mDefaultCommandPool = std::make_shared<VulkanCommandPool>(surface, physicalDevice, device, identifier);
    }
    void SetupDebugMessenger();

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
        mSwapChain->EndFrameDrawing(graphicsQueue, *(mDefaultCommandPool->CommandBuffers()[mSwapChain->CurrentFrame()]), presentQueue, framebufferResized, currentImage);
    }

    Ptr(VulkanCommandBuffer) GetFrameCommandBuffer()
    {
        return mDefaultCommandPool->CommandBuffers()[mSwapChain->CurrentFrame()];
    }

    Ptr(VulkanGraphicsPipeline) PrimaryGraphicsPipeline()
    {
        return mGraphicsPipeline;
    }

    Ptr(VulkanDescriptorLayout) DescriptorHandler()
    {
        return mDescriptorHandler;
    }

    VkRenderPass RenderPass()
    {
        return renderPass;
    }

    Ptr(VulkanSwapChain) SwapChain()
    {
        return mSwapChain;
    }

    // Cleanup the swap chain.
    void cleanupSwapChain() {

        vkDestroyImageView(device, mSwapChain->DepthImageView(), nullptr);
        vkDestroyImage(device, mSwapChain->DepthImage(), nullptr);
        vkFreeMemory(device, mSwapChain->DepthImageMemory(), nullptr);

        for (auto framebuffer : mSwapChain->FrameBuffers()) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        mDefaultCommandPool->FreeCommandBuffers(device);

        // Destroy the image views.
        for (auto imageView : mSwapChain->ImageViews()) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        // Destroy the swap chain.
        vkDestroySwapchainKHR(device, mSwapChain->SwapChain(), nullptr);

        vkDestroyDescriptorPool(device, mDescriptorHandler->BuiltDescriptorPool(), nullptr);
    }

    void cleanup() {

        cleanupSwapChain();

        mGraphicsPipeline->CleanupPipeline(device);
        vkDestroyRenderPass(device, renderPass, nullptr);

        vkDestroyDescriptorSetLayout(device, mDescriptorHandler->Layout(), nullptr);

            

        // Cleanup the syncronization objects for the frames.
        mSwapChain->CleanUp();

        mDefaultCommandPool->DestroyCommandPool(device);

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
        mBufferUtilities = std::make_shared<VulkanBufferUtilities>(physicalDevice, device, mDefaultCommandPool->CommandPool(), graphicsQueue);
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
        mSwapChain->CreateDepthImage(physicalDevice, device);
        mSwapChain->CreateFrameBuffers(device, renderPass);
        CreateUniformBuffers();
        CreateDefaultRenderCommandBuffers(nullptr, nullptr, 0); // TODO:: Fix
        */
    }

    // Create objects needed for syncronization.
    //void CreateSyncObjects();

    void CreateDefaultRenderCommandBuffers() {
        for (size_t i = 0; i < mSwapChain->FrameBuffers().size(); i++) {
            auto commandBuffer = mDefaultCommandPool->CreateCommandBuffer(device);
        }
    }

    private:
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
};


#endif
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
#include <functional>

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

struct VulkanAutoInitSettings
{
    VulkanInstanceInfo InstanceInfo;
    int WindowWidth;
    int WindowHeight;
    std::string WindowName;
    bool SetupDebug;
    SwapChainDescriptor SwapChainDescriptor;
};



class VulkanRenderer : public VulkanPipelineHolderIntf {
// ------------------------------------------------------------------------------------------------------------------
public: // VulkanPipelineHolderIntf Implementation
 // ------------------------------------------------------------------------------------------------------------------
    operator VkPhysicalDevice () override
    {
        return mPhysicalDevice;
    }

    operator VkDevice () override
    {
        return mDevice;
    }

    operator VkQueue () override
    {
        return mDefaultGraphicsQueue;
    }

    operator VkCommandPool () override
    {
        return mDefaultCommandPool->CommandPool();
    }

// ------------------------------------------------------------------------------------------------------------------
public: // Public Member Variables
// ------------------------------------------------------------------------------------------------------------------
    /*
    
        Here contain some of the basic universal vulkan types.

        These can be manually changed if desired as long as it is done during the proper initalization phase.
    
    */
    GLFWwindow* mWindow;
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;
    // Native surface.
    VkSurfaceKHR mSurface;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    // Logical Device:
    VkDevice mDevice;
    // Default Queues
    VkQueue mDefaultGraphicsQueue;
    VkQueue mPresentQueue;

    // Default Rener Pass
    VkRenderPass mRenderPass;

    // A flag that triggers when the window is resized.
    bool framebufferResized = false;


    /*
        Custom Handlers,
        These can also be manually initalized or even subclassed.
    */
    std::shared_ptr<VulkanGraphicsPipeline> mGraphicsPipeline;
    std::shared_ptr<VulkanSwapChain> mSwapChain;
    std::shared_ptr<VulkanDescriptorLayout> mDescriptorHandler;
    std::shared_ptr<VulkanBufferUtilities> mBufferUtilities;
    // Manages allocation of Command Buffers.
    std::shared_ptr<VulkanCommandPool> mDefaultCommandPool;

// ------------------------------------------------------------------------------------------------------------------
public: // Public Methods
// ------------------------------------------------------------------------------------------------------------------
  
    // ------------------------------------------------------------------------------------------------------------------
    // Initalizers
    // ------------------------------------------------------------------------------------------------------------------
    // These initalizes are separated so you can create custom objects in different stages
    // if desired. Call AutoInitialize() if you don't want anything but the minimal amount of customization.

    void CreateGLFWWindow(int width, int height, std::string name);
    void CreateVulkanInstance(VulkanInstanceInfo instanceInfo);
    void CreateGLFWSurface();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void SetupSwapChain(const SwapChainDescriptor descriptor);
    void CreateRenderPass();
    void CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor);
    void CreateDefaultCommandPool(std::string identifier) {
        mDefaultCommandPool = std::make_shared<VulkanCommandPool>(mSurface, mPhysicalDevice, mDevice, identifier);
    }
    void SetupDebugMessenger();

    /// <summary>
    /// This function will handle the default initialization of the renderer for you.
    /// 
    /// Define custom features of the renderer using the provided setting structs and function.
    /// </summary>
    /// <param name="settings">The settings of the program.</param>
    /// <param name="descriptorLayoutBuilderStage">Create the descriptor layout for your main shader.</param>
    /// <param name="pipelineDescriptionStage">Create the graphics pipeline descriptor.</param>
    /// <param name="loadingStage">A generic loading stage to load buffers.</param>
    /// <param name="descriptorSetCreationStage">Define your descriptor sets.</param>
    void AutoInitialize
    (
        VulkanAutoInitSettings settings,
        std::function<void(Ptr(VulkanDescriptorLayout))> descriptorLayoutBuilderStage,
        std::function<GraphicsPipelineDescriptor()> pipelineDescriptionStage,
        std::function<void()> loadingStage,
        std::function<void(Ptr(VulkanDescriptorSetBuilder))> descriptorSetCreationStage
    );

    // ------------------------------------------------------------------------------------------------------------------
    // Frame Drawing Utilities
    // ------------------------------------------------------------------------------------------------------------------

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
        mSwapChain->EndFrameDrawing(mDefaultGraphicsQueue, *(mDefaultCommandPool->CommandBuffers()[mSwapChain->CurrentFrame()]), mPresentQueue, framebufferResized, currentImage);
    }

    Ptr(VulkanCommandBuffer) GetFrameCommandBuffer()
    {
        return mDefaultCommandPool->CommandBuffers()[mSwapChain->CurrentFrame()];
    }

    // ------------------------------------------------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------------------------------------------------

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
        return mRenderPass;
    }

    Ptr(VulkanSwapChain) SwapChain()
    {
        return mSwapChain;
    }

    // ------------------------------------------------------------------------------------------------------------------
    // Cleanup the program
    // ------------------------------------------------------------------------------------------------------------------

    // Cleanup the swap chain.
    void cleanupSwapChain() {

        vkDestroyImageView(mDevice, mSwapChain->DepthImageView(), nullptr);
        vkDestroyImage(mDevice, mSwapChain->DepthImage(), nullptr);
        vkFreeMemory(mDevice, mSwapChain->DepthImageMemory(), nullptr);

        for (auto framebuffer : mSwapChain->FrameBuffers()) {
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
        }

        mDefaultCommandPool->FreeCommandBuffers(mDevice);

        // Destroy the image views.
        for (auto imageView : mSwapChain->ImageViews()) {
            vkDestroyImageView(mDevice, imageView, nullptr);
        }

        // Destroy the swap chain.
        vkDestroySwapchainKHR(mDevice, mSwapChain->SwapChain(), nullptr);

        vkDestroyDescriptorPool(mDevice, mDescriptorHandler->BuiltDescriptorPool(), nullptr);
    }

    void cleanup() {

        cleanupSwapChain();

        mGraphicsPipeline->CleanupPipeline(mDevice);
        vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

        vkDestroyDescriptorSetLayout(mDevice, mDescriptorHandler->Layout(), nullptr);

            

        // Cleanup the syncronization objects for the frames.
        mSwapChain->CleanUp();

        mDefaultCommandPool->DestroyCommandPool(mDevice);

        // Destroy the device.
        vkDestroyDevice(mDevice, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        vkDestroyInstance(mInstance, nullptr);

        // Destroy and terminate the window.
        glfwDestroyWindow(mWindow);
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
        mBufferUtilities = std::make_shared<VulkanBufferUtilities>(mPhysicalDevice, mDevice, mDefaultCommandPool->CommandPool(), mDefaultGraphicsQueue);
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
            auto commandBuffer = mDefaultCommandPool->CreateCommandBuffer(mDevice);
        }
    }
// ------------------------------------------------------------------------------------------------------------------
 private: // Private Methods
// ------------------------------------------------------------------------------------------------------------------

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
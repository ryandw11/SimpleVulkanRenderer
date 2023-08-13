#include "VulkanSwapChain.hpp"
#include "VulkanRendererTypes.hpp"
#include "VulkanImageUtilities.hpp"

#include <stdexcept>
#include <array>

namespace
{
    /// <summary>
    /// Choose the best swap surfact format.
    /// 
    /// The default impl wants VK_FORMAT_B8G8R8A8_SRGB as the format and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR as the color space.
    /// </summary>
    /// <param name="availableFormats">The list of formats available for use.</param>
    /// <returns>The "best" format. </returns>
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(VkPresentModeKHR preferedPresentationMode, const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == preferedPresentationMode)
            {
                return availablePresentMode;
            }
        }

        // Safe and guareented to exist. No screen tearing but could cause latenecy (like vsync).
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D CreateSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            // Clamp the value of width and height between the allowed minimum and maximum extents that are supported
            // by the implementation.
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }
}

VulkanSwapChain::VulkanSwapChain(const SwapChainDescriptor descriptor)
	: mDescriptor(descriptor)
{
}

void VulkanSwapChain::InitializeSwapChain(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(mDescriptor.PresentationMode, swapChainSupport.presentModes);
    VkExtent2D extent = CreateSwapExtent(window, swapChainSupport.capabilities);

    // Number of images that are in the swap chain
    uint32_t imageCount = mDescriptor.ImageCount ? *mDescriptor.ImageCount : swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indicies = FindQueueFamilies(surface, physicalDevice);
    uint32_t queueFamilyIndicies[] = { indicies.graphicsFamily.value(), indicies.presentFamily.value() };

    // Choose the image sharing mode based on if the graphics and presentation queues are the same or not.
    if (indicies.graphicsFamily != indicies.presentFamily)
    {
        // Concurrent means images can be owned and used by multiple queues.
        // Not the best performance, but does not require explicit movement of images from queue to queue.
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicies;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // If the alpha channel should be used for blending with other windows in the window system.
    // This specifies to ignore the alpha channel.
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    // We don't care about the color of the pixels that are obscured.
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create it using the device, swap chain info, and the place to store the swap chain.
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create valid swap chain!");
    }

    vkGetSwapchainImagesKHR(device, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, mSwapChain, &imageCount, mSwapChainImages.data());

    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;

    CreateImageViews(device);
}

void VulkanSwapChain::CreateDepthImage(VkPhysicalDevice physicalDevice, VkDevice device)
{
    VkFormat depthFormat = FindDepthFormat(physicalDevice);
    CreateImage(physicalDevice, device, mSwapChainExtent.width, mSwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mDepthImage, mDepthImageMemory);
    mDepthImageView = CreateImageView(device, mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanSwapChain::CreateFrameBuffers(VkDevice device, VkRenderPass renderPass)
{
    // Resize the container to hold all of the frame buffers.
    mSwapChainFrameBuffers.resize(mSwapChainImageViews.size());

    for (size_t i = 0; i < mSwapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
            mSwapChainImageViews[i],
            mDepthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass; // For now, lets handle a single render pass.
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = mSwapChainExtent.width;
        framebufferInfo.height = mSwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create freambuffer!");
        }
    }
}

void VulkanSwapChain::CreateImageViews(VkDevice device)
{
    // Resize to be the same size as the number of swap chain images.
    mSwapChainImageViews.resize(mSwapChainImages.size());

    for (size_t i = 0; i < mSwapChainImages.size(); i++)
    {
        mSwapChainImageViews[i] = CreateImageView(device, mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

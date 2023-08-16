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

VulkanSwapChain::VulkanSwapChain(VkDevice device, const SwapChainDescriptor descriptor)
	: 
    mDevice(device),
    mDescriptor(descriptor),
    mCurrentFrame(0)
{
}

void VulkanSwapChain::InitializeSwapChain(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice)
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
    if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create valid swap chain!");
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;

    CreateImageViews();
}

void VulkanSwapChain::CreateDepthImage(VkPhysicalDevice physicalDevice)
{
    VkFormat depthFormat = FindDepthFormat(physicalDevice);
    CreateImage(physicalDevice, mDevice, mSwapChainExtent.width, mSwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mDepthImage, mDepthImageMemory);
    mDepthImageView = CreateImageView(mDevice, mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanSwapChain::CreateFrameBuffers(VkRenderPass renderPass)
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

        if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create freambuffer!");
        }
    }
}

void VulkanSwapChain::CreateSyncObjects()
{
    // TODO:: Replace with max frames in flight.
    mImageAvailableSemaphore = VulkanFrameObject<VkSemaphore>(2);
    mRenderFinishedSemaphore = VulkanFrameObject<VkSemaphore>(2);
    mInFlightFence = VulkanFrameObject<VkFence>(2);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < 2; i++)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFence[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

uint32_t VulkanSwapChain::StartFrameDrawing()
{
    vkWaitForFences(mDevice, 1, &mInFlightFence[mCurrentFrame], VK_TRUE, UINT64_MAX); // Ensure the frame is available and not being processed by the GPU.
    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphore[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // TODO: Figure out what to do about this.
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to quire swapchain image during frame drawing.");
    }

    vkResetFences(mDevice, 1, &mInFlightFence[mCurrentFrame]);

    return imageIndex;
}

void VulkanSwapChain::EndFrameDrawing(VkQueue graphicsQueue, VkCommandBuffer commandBuffer, VkQueue presentationQueue, bool& framebufferResized, uint32_t imageIndex)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore[mCurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore[mCurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, mInFlightFence[mCurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(presentationQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        //recreateSwapChain();
        // TODO: Create swap chain.
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image.");
    }

    // TODO:: Update this "2" to be max frames in flight.
    mCurrentFrame = (mCurrentFrame + 1) % 2;
}

void VulkanSwapChain::CreateImageViews()
{
    // Resize to be the same size as the number of swap chain images.
    mSwapChainImageViews.resize(mSwapChainImages.size());

    for (size_t i = 0; i < mSwapChainImages.size(); i++)
    {
        mSwapChainImageViews[i] = CreateImageView(mDevice, mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

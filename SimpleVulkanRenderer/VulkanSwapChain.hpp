#pragma once
#ifndef VULKAN_SWAP_CHAIN_H
#define VULKAN_SWAP_CHAIN_H

#include <optional>

#include "VulkanIncludes.hpp"
#include "VulkanFrameObject.hpp"

struct SwapChainDescriptor
{
	/// <summary>
	/// The number of images the swap chain should use.
	/// </summary>
	std::optional<int> ImageCount;
	/// <summary>
	/// The prefered presentation mode to select.
	/// If the selected mode is not supported, VK_PRESENT_MODE_FIFO_KHR is selected instead.
	/// </summary>
	VkPresentModeKHR PresentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
};

/// <summary>
/// This class manages the swap chain and everything that goes with it.
/// 
/// The swap chain, its images, framebuffers, and depth image are all included in this class.
/// </summary>
class VulkanSwapChain
{
public:
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static VulkanSwapChain::SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		VulkanSwapChain::SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Querying the supported surface formats.
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// Querying the supported presentation modes.
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

public:
	VulkanSwapChain(VkDevice device, const SwapChainDescriptor descriptor);

	void InitializeSwapChain(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);

	void CreateDepthImage(VkPhysicalDevice physicalDevice);

	void CreateFrameBuffers(VkRenderPass renderPass);

	void CreateSyncObjects();

	/// <summary>
	/// Start the drawing of a frame, CurrentFrame() should already
	/// represent the frame you want to start drawing on.
	/// </summary>
	/// TODO :: Handle swapchain recreation.
	uint32_t StartFrameDrawing();

	/// <summary>
	/// Ends frame drawing and tells the frame
	/// command buffer to be submited.
	/// </summary>
	void EndFrameDrawing(VkQueue graphicsQueue, VkCommandBuffer commandBuffer, VkQueue presentationQueue, bool& framebufferResized, uint32_t imageIndex);

	// TODO
	//void CleanUp();

	// Getters

	const size_t CurrentFrame()
	{
		return mCurrentFrame;
	}

	const VkSwapchainKHR SwapChain()
	{
		return mSwapChain;
	}

	const VkExtent2D& Extent()
	{
		return mSwapChainExtent;
	}

	const std::vector<VkFramebuffer>& FrameBuffers()
	{
		return mSwapChainFrameBuffers;
	}

	const VkFormat& ImageFormat()
	{
		return mSwapChainImageFormat;
	}

	const std::vector<VkImage>& Images()
	{
		return mSwapChainImages;
	}

	const std::vector<VkImageView>& ImageViews()
	{
		return mSwapChainImageViews;
	}

	const VkImage DepthImage()
	{
		return mDepthImage;
	}

	const VkImageView DepthImageView()
	{
		return mDepthImageView;
	}

	const VkDeviceMemory DepthImageMemory()
	{
		return mDepthImageMemory;
	}


private:
	void CreateImageViews();

private:
	const SwapChainDescriptor mDescriptor;

	VkSwapchainKHR mSwapChain;
	VkExtent2D mSwapChainExtent;

	VkFormat mSwapChainImageFormat;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;

	std::vector<VkFramebuffer> mSwapChainFrameBuffers;

	VulkanFrameObject<VkSemaphore> mImageAvailableSemaphore;
	VulkanFrameObject<VkSemaphore> mRenderFinishedSemaphore;
	VulkanFrameObject<VkFence> mInFlightFence;

	VkImage mDepthImage;
	VkImageView mDepthImageView;
	VkDeviceMemory mDepthImageMemory;

	// Current State:
	size_t mCurrentFrame;
	uint32_t mCurrentImageIndex;


	// General Pipline Info Storage
	VkDevice mDevice;
};

#endif
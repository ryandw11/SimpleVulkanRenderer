#pragma once
#ifndef VULKAN_COMMAND_POOL_H
#define VULKAN_COMMAND_POOL_H

#include <string>
#include <thread>
#include <optional>

#include "VulkanIncludes.hpp"
#include "VulkanRendererTypes.hpp"

class VulkanCommandBuffer;

// TODO :: Setup so these can be reset or re-crated.
class VulkanCommandPool
{
public:
	VulkanCommandPool(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, std::string identifier, std::optional<VulkanQueue> vulkanQueue = std::nullopt);

	std::shared_ptr<VulkanCommandBuffer> CreateCommandBuffer(VkDevice device);

	void FreeCommandBuffers(VkDevice device);
	void DestroyCommandPool(VkDevice device);

	const std::thread::id OwningThread() const
	{
		return mOwningThread;
	}

	const std::vector<std::shared_ptr<VulkanCommandBuffer>>& CommandBuffers()
	{
		return mCommandBuffers;
	}

	const VkCommandPool CommandPool()
	{
		return mCommandPool;
	}

private:
	const std::string mIdentifier;
	const std::thread::id mOwningThread;

	VkCommandPool mCommandPool;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> mCommandBuffers;
};

#endif
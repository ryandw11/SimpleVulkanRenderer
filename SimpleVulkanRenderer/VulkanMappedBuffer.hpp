#pragma once
#ifndef VULKAN_MAPPED_BUFFER_H
#define VULKAN_MAPPED_BUFFER_H

#include "VulkanBuffer.hpp"

/// <summary>
/// A vulkan buffer that always has an active memory map.
/// </summary>
class VulkanMappedBuffer : public VulkanBuffer
{
public:
	VulkanMappedBuffer() 
		: 
		VulkanBuffer(), 
		mMemoryData(nullptr)
	{
	}

	VulkanMappedBuffer(VkBuffer buffer, VkDeviceMemory deviceMemory)
		: VulkanBuffer(buffer, deviceMemory), mMemoryData(nullptr)
	{}

	void* MappedMemory()
	{
		return mMemoryData;
	}

	void** DirectMappedMemory()
	{
		return &mMemoryData;
	}
private:
	void* mMemoryData;
};

#endif
#pragma once
#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include "VulkanIncludes.hpp"

/// <summary>
/// A wrapper for the VkBuffer and the vKDeviceMemory that goes with it.
/// 
/// This wrapper simply wraps the respective pointers. Keep in mind when passing
/// by value that this wrapper can become out of date if you are not refering to your main instance.
/// 
/// The VkBuffer is not destroyed when this class is deconstructed. #DestroyBuffer() must be called
/// to free both the buffer and device memory.
/// </summary>
class VulkanBuffer
{
public:
	VulkanBuffer();
	VulkanBuffer(VkBuffer buffer, VkDeviceMemory deviceMemory);

	operator VkBuffer() const
	{
		return mInternalBuffer;
	}

	operator VkBuffer&()
	{
		return mInternalBuffer;
	}

	operator VkDeviceMemory() const
	{
		return mInternalMemory;
	}

	operator VkDeviceMemory&()
	{
		return mInternalMemory;
	}

	operator bool() const
	{
		return Initialized();
	}

	bool Initialized() const
	{
		return mInternalBuffer != nullptr && mInternalMemory != nullptr;
	}

	void DestoryBuffer(VkDevice device);

private:
	VkBuffer mInternalBuffer;
	VkDeviceMemory mInternalMemory;
};

#endif
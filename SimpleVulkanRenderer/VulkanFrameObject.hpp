#pragma once
#ifndef VULKAN_FRAME_OBJECT_H
#define VULKAN_FRAME_OBJECT_H

#include <vector>
#include <stdexcept>

#include "VulkanIncludes.hpp"

/// <summary>
/// Represents a vulkan object where one is needed per frame buffer.
/// 
/// Examples may include:
/// 1. Buffers for uniforms
/// 2. Image Views and Samplers
/// 3. Semaphores/Fences for synchronization
/// </summary>
/// <typeparam name="T">The parent vulkan type. This should be a pointer.</typeparam>
template<typename T>
class VulkanFrameObject {
public:
	VulkanFrameObject()
		: mInternalVector(), mFromSingle(false)
	{}

	/// <summary>
	/// Initalize with an object for each frame.
	/// </summary>
	/// <param name="objects">The vector of already created objects. (Must be the same length as the number of frames)</param>
	VulkanFrameObject(std::vector<T> objects)
		:
		mInternalVector(objects),
		mFromSingle(false)
	{
	}

	/// <summary>
	/// Initalize with the number of frames.
	/// 
	/// Internally the resize() function is called. Objects
	/// can be set by referencing each once.
	/// </summary>
	/// <param name="frames">The number of frames.</param>
	VulkanFrameObject(size_t frames)
		:
		mFromSingle(false)
	{
		mInternalVector.resize(frames);
	}

	/// <summary>
	/// Initalize with an object that remains the same for all frames.
	/// </summary>
	/// <param name="object">The object.</param>
	VulkanFrameObject(T object)
		:
		mInternalVector({ object }),
		mFromSingle(true)
	{
	}

	T& operator [](uint32_t frame)
	{
		if (mFromSingle)
		{
			return mInternalVector[0];
		}
		else
		{
			if (frame >= mInternalVector.size())
			{
				throw std::runtime_error("Object count does not match the number of frame buffers!");
			}
			return mInternalVector[frame];
		}
	}

	const std::vector<T>& InternalVector()
	{
		return mInternalVector;
	}
private:
	std::vector<T> mInternalVector;
	bool mFromSingle;
};

typedef VulkanFrameObject<VkBuffer> VulkanFrameBuffer;
typedef VulkanFrameObject<VkImageView> VulkanFrameImageView;
typedef VulkanFrameObject<VkSampler> VulkanFrameSampler;

#endif
#pragma once
#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include <string>

#include "VulkanIncludes.hpp"

class VulkanBufferUtilities;
class VulkanPipelineHolderIntf;

/// <summary>
/// Represents a texture that will be used in rendering.
/// 
/// Mainly used to be a sampler in the fragment shader.
/// </summary>
class VulkanTexture
{
public:
	VulkanTexture(std::string texturePath, Ptr(VulkanPipelineHolderIntf) pipelineHolder, Ptr(VulkanBufferUtilities) bufferUtilities);

	void DestroyTexture(VkDevice device);

	const VkImage Image()
	{
		return mTextureImage;
	}

	const VkImageView ImageView()
	{
		return mTextureImageView;
	}

	const VkDeviceMemory DeviceMemory()
	{
		return mTextureMemory;
	}

	const VkSampler Sampler()
	{
		return mTextureSampler;
	}

private:
	std::string mTexturePath;
	int mTextureWidth;
	int mTextureHeight;
	int mTextureChannels;

	VkImage mTextureImage;
	VkImageView mTextureImageView;
	VkDeviceMemory mTextureMemory;
	VkSampler mTextureSampler;
};
#endif
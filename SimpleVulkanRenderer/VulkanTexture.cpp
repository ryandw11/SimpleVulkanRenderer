#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanBufferUtilities.hpp"
#include "VulkanImageUtilities.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPipelineHolderIntf.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace
{
}

VulkanTexture::VulkanTexture(std::string texturePath, Ptr(VulkanPipelineHolderIntf) pipelineHolder, Ptr(VulkanBufferUtilities) bufferUtilities)
	:
	mTexturePath(texturePath)
{
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &mTextureWidth, &mTextureHeight, &mTextureChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = mTextureWidth * mTextureHeight * 4 /*RGBA*/;

	if (!pixels)
	{
		throw std::runtime_error("Failed to load image!");
	}

	VulkanBuffer stagingBuffer = bufferUtilities->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(*pipelineHolder, stagingBuffer, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(*pipelineHolder, stagingBuffer);

	stbi_image_free(pixels);

	CreateImage(
		*pipelineHolder,
		*pipelineHolder,
		mTextureWidth,
		mTextureHeight,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mTextureImage,
		mTextureMemory);

	// Transition Image
	auto commandBuffer = CreateSingleUseCommandBuffer(*pipelineHolder, *pipelineHolder);
	commandBuffer->TransitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	commandBuffer->SubmitSingleUseCommand(*pipelineHolder, *pipelineHolder);

	// Copy Buffer
	commandBuffer = CreateSingleUseCommandBuffer(*pipelineHolder, *pipelineHolder);
	commandBuffer->CopyBufferToImage(stagingBuffer, mTextureImage, static_cast<uint32_t>(mTextureWidth), static_cast<uint32_t>(mTextureHeight));
	commandBuffer->SubmitSingleUseCommand(*pipelineHolder, *pipelineHolder);

	// Transition again
	commandBuffer = CreateSingleUseCommandBuffer(*pipelineHolder, *pipelineHolder);
	commandBuffer->TransitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	commandBuffer->SubmitSingleUseCommand(*pipelineHolder, *pipelineHolder);

	stagingBuffer.DestoryBuffer(*pipelineHolder);

	// Texture View
	mTextureImageView = CreateImageView(*pipelineHolder, mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	// Texture Sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	// Hoe to iterpolate texels (pixels) that are magnified or minified.
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	// Axes are called U, V, W instead of X, Y, Z.
	// Describes what the texture should do, such as reapeat, clamp to edge, clamp to border, mirror repeat, etc.
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// Enable anisotropy to prevent the bluring of the texture when there are too many texels vs fragments.
	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(*pipelineHolder, &properties);
	// Get the max anisotrophy supported by the physical device (the gpu) itself.
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	// The color that shows when you sample outside of the texture. Black, White, or Transparent are options. No other colors allowed.
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	// Coordinates should be normalized between 0-1
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(*pipelineHolder, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler!");
	}
}

void VulkanTexture::DestroyTexture(VkDevice device)
{
	vkDestroySampler(device, mTextureSampler, nullptr);
	vkDestroyImageView(device, mTextureImageView, nullptr);
	vkDestroyImage(device, mTextureImage, nullptr);
	vkFreeMemory(device, mTextureMemory, nullptr);
}

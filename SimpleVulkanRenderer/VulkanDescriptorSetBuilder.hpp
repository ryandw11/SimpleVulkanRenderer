#pragma once
#ifndef VULKAN_DESCRIPTOR_SET_H
#define VULKAN_DESCRIPTOR_SET_H

#include <optional>

#include "VulkanIncludes.hpp"
#include "VulkanFrameObject.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanMappedBuffer.hpp"

class VulkanDescriptorSetBuilder
{
public:
	struct FrameDescriptorBufferInfo
	{
		VulkanFrameBuffer FrameBuffer;
		VkDeviceSize Range;
	};
	struct FrameDescriptorImageInfo
	{
		VulkanFrameImageView FrameImageView;
		VulkanFrameSampler FrameSampler;
	};
	struct DescriptorSetInfo
	{
		std::optional<FrameDescriptorBufferInfo> BufferInfo;
		std::optional<FrameDescriptorImageInfo> ImageInfo;
		bool IsBuffer;
		uint32_t DstBinding;
		uint32_t DstArrayElement;
		VkDescriptorType DescriptorType;

	};
public:
	VulkanDescriptorSetBuilder(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t setCount);

	void DescribeBuffer(uint32_t binding, uint32_t arrayElement, VulkanFrameBuffer buffer, VkDeviceSize range);
	void DescribeBuffer(uint32_t binding, uint32_t arrayElement, VulkanFrameObject<VulkanBuffer> buffer, VkDeviceSize range);
	void DescribeBuffer(uint32_t binding, uint32_t arrayElement, VulkanFrameObject<VulkanMappedBuffer> buffer, VkDeviceSize range);
	void DescribeImageSample(uint32_t binding, uint32_t arrayElement, VulkanFrameImageView imageView, VulkanFrameSampler sampler);

	std::vector<VkDescriptorSet> UpdateDescriptorSets();

	const std::vector<VkDescriptorSet> GetBuiltDescriptorSets()
	{
		return mDescriptorSets;
	}

private:
	VkDevice mDevice;

	std::vector<DescriptorSetInfo> mSetInfos;

	std::vector<VkDescriptorSet> mDescriptorSets;
};

#endif
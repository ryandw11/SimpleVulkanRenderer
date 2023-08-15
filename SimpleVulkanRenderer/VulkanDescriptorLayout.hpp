#pragma once
#ifndef VULKAN_DESCRIPTOR_LAYOUT_H
#define VULKAN_DESCRIPTOR_LAYOUT_H

#include <memory>

#include "VulkanIncludes.hpp"
#include "VulkanDescriptorSetBuilder.hpp"

/// <summary>
/// A descriptor layout describes a global resource that the shaders can use.
/// 
/// Use this class to describe and create a DescriptorLayout.
/// Then, create a descriptor pool and descriptor set when needed.
///
/// General Flow:
/// 1. Build Descriptor Layout
/// ... (Create Graphics Pipeline, commands pools, and needed buffers)
/// 2. Build Descriptor Set Pool
/// 3. Build required Descriptor Sets
/// </summary>
class VulkanDescriptorLayout
{
public:
	struct DescriptorSetLayoutBindingInfo
	{
		VkDescriptorType type;
		VkDescriptorSetLayoutBinding binding;
	};
public:
	VulkanDescriptorLayout(VkDevice device);

	// ---------------------------------------------------
	// Defining Descriptor Layout
	// ---------------------------------------------------

	void UniformBufferBinding(uint32_t binding, uint32_t count = 1, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS);
	void ImageSamplerBinding(uint32_t binding, uint32_t count = 1, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS);
	void GenericLayoutBinding(VkDescriptorSetLayoutBinding binding, VkDescriptorType type);
	VkDescriptorSetLayout BuildLayout();
	bool IsBuilt();

	// ---------------------------------------------------
	// Descriptor Pool Creation
	// ---------------------------------------------------
	
	/// <summary>
	/// Create a descriptor pool. This handles the memory allocation of the descriptor sets.
	/// 
	/// Create this before using the set builder!
	/// </summary>
	/// <param name="descriptorSetCount">The number of descriptor sets to create.
	///		For example: You would want one descriptor set per framebuffer.
	/// </param>
	/// <returns>The descriptor pool.</returns>
	VkDescriptorPool CreateDescriptorPool(uint32_t descriptorSetCount);

	/// <summary>
	/// Get the builder to update descriptor sets.
	/// 
	/// Nullptr unless a descriptor pool has been created.
	/// </summary>
	/// <returns>The pointer to a builder.</returns>
	std::shared_ptr<VulkanDescriptorSetBuilder> DescriptorSetBuilder();

	/// <summary>
	/// Access the built descriptor pool.
	/// </summary>
	/// <returns>The built descriptor pool.</returns>
	const VkDescriptorPool BuiltDescriptorPool()
	{
		ValidateNonNull(mBuiltPool, "The descriptor pool has not yet been built!");

		return mBuiltPool;
	}

	const VkDescriptorSetLayout Layout()
	{
		ValidateNonNull(mLayout, "Attempted to get the layout, but it was null!");

		return mLayout;
	}

private:
	void ValidateLayoutNotYetBuilt();
	void AddBinding(VkDescriptorType type, VkDescriptorSetLayoutBinding binding);

private:
	VkDevice mDevice;

	VkDescriptorSetLayout mLayout;
	std::vector<DescriptorSetLayoutBindingInfo> mLayoutBindings;

	VkDescriptorPool mBuiltPool;
	std::shared_ptr<VulkanDescriptorSetBuilder> mSetBuilder;
};

#endif
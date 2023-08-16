#pragma once

#ifndef VULKAN_GRAPHICS_PIPELINE
#define VULKAN_GRAPHICS_PIPELINE

#include <memory>
#include <string>

#include "VulkanIncludes.hpp"
#include "VulkanVertexShader.hpp"
#include "VulkanFragmentShader.hpp"

struct GraphicsPipelineDescriptor
{
	std::shared_ptr<VulkanVertexShader> VertexShader;
	std::shared_ptr<VulkanFragmentShader> FragmentShader;
	std::vector<std::shared_ptr<VulkanShaderIntf>> OtherShaders;

	VkPrimitiveTopology InputTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
	float LineWidth = 1.0f;
	VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
	VkFrontFace VertexOrder = VK_FRONT_FACE_COUNTER_CLOCKWISE;
};

class VulkanGraphicsPipeline
{
public:
	VulkanGraphicsPipeline(const GraphicsPipelineDescriptor descriptor);

	void UpdatePipeline(VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void CleanupPipeline(VkDevice device);

	VkPipeline Pipeline() const;
	VkPipelineLayout PipelineLayout() const;

private:
	VkPipeline mPipeline;
	VkPipelineLayout mPipelineLayout;
	const GraphicsPipelineDescriptor mDescriptor;

	std::shared_ptr<VulkanVertexShader> mVertexShader;
	std::shared_ptr<VulkanFragmentShader> mFragmentShader;
	std::vector<std::shared_ptr<VulkanShaderIntf>> mOtherShaders;
};

#endif
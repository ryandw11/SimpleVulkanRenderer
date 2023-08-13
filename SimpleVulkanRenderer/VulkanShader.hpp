#pragma once
#ifndef VULKAN_SHADER_H
#define VULKAN_SHADER_H

#include "VulkanIncludes.hpp"

class VulkanShaderIntf {
public:
	~VulkanShaderIntf() = default;

	virtual VkPipelineShaderStageCreateInfo GetShaderStage() = 0;

	/// <summary>
	/// Destroy the shader module if needed.
	/// 
	/// This should only be called by the GraphicsPipeline class.
	/// </summary>
	virtual void DestroyShaderModuleIfNeeded(VkDevice device) = 0;
};

#endif
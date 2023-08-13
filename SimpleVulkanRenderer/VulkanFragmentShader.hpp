#pragma once

#ifndef VULKAN_FRAGMENT_SHADER
#define VULKAN_FRAGMENT_SHADER

#include <string>

#include "VulkanShader.hpp"
#include "VulkanIncludes.hpp"

class VulkanFragmentShader : public VulkanShaderIntf
{
public:
	VulkanFragmentShader(VkDevice device, std::string startingFunctionName, std::string filePath);

	/// <summary>
	/// Get the current shader stage.
	/// </summary>
	/// <returns>The current shader stage.</returns>
	VkPipelineShaderStageCreateInfo GetShaderStage() override;
	void DestroyShaderModuleIfNeeded(VkDevice device) override;


private:
	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo;
	VkShaderModule mFragmentShaderModule;

	std::string mFunctionStartName;
};

#endif
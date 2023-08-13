#pragma once

#ifndef VULKAN_VERTEX_SHADER
#define VULKAN_VERTEX_SHADER

#include <string>

#include "VulkanShader.hpp"
#include "VulkanIncludes.hpp"

class VulkanVertexShader : public VulkanShaderIntf
{
public:
	VulkanVertexShader(VkDevice device, std::string startingFunctionName, std::string filePath);

	/// <summary>
	/// Get the current shader stage.
	/// </summary>
	/// <returns>The current shader stage.</returns>
	VkPipelineShaderStageCreateInfo GetShaderStage() override;

	void DestroyShaderModuleIfNeeded(VkDevice device) override;

	/// <summary>
	/// Define a vertex attribute for the shader.
	/// </summary>
	/// <param name="binding">The binding this attribute refers to.</param>
	/// <param name="location">The location of the attribute.</param>
	/// <param name="format">The format of the attribute.</param>
	/// <param name="offset">The offset of the attribute</param>
	void VertexAttribute(uint32_t binding, uint32_t location, VkFormat format = VK_FORMAT_R32G32B32_SFLOAT, uint32_t offset = 0);

	/// <summary>
	/// Define a uniform binding for the shader.
	/// </summary>
	/// <param name="bindingNumber">The binding number.</param>
	/// <param name="stride">The stride (size of the objects passed)</param>
	/// <param name="inputRate">The input rate. (Defaults to per vertex)</param>
	void VertexUniformBinding(uint32_t bindingNumber, uint32_t stride, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

	/// <summary>
	/// Get the Vertex Input State Create Info struct.
	/// </summary>
	/// <returns>The Vertex Input State Create Info struct.</returns>
	VkPipelineVertexInputStateCreateInfo GetVertexInputStateInfo();


private:
	VkPipelineShaderStageCreateInfo vertexShaderStageInfo;
	VkShaderModule mVertexShaderModule;
	std::string mFunctionStartName;

	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription> bindings;
};

#endif
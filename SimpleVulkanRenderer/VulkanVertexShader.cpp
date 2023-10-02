#include "VulkanVertexShader.hpp"

#include <iostream>
#include <fstream>

namespace
{
    std::vector<char> readFile(const std::string& filename) {
        // Open the file starting at the end.
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}

VulkanVertexShader::VulkanVertexShader(VkDevice device, std::string name, std::string file) : mFunctionStartName(name)
{
    auto vertexShaderCode = readFile(file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = vertexShaderCode.size();
    // The api takes in a uint32_t instead of a character pointer for byte data.
    createInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderCode.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &mVertexShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = mVertexShaderModule;
    vertexShaderStageInfo.pName = mFunctionStartName.c_str();

    this->vertexShaderStageInfo = vertexShaderStageInfo;
}

VkPipelineShaderStageCreateInfo VulkanVertexShader::GetShaderStage()
{
    return vertexShaderStageInfo;
}

void VulkanVertexShader::DestroyShaderModuleIfNeeded(VkDevice device)
{
    if (mVertexShaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device, mVertexShaderModule, nullptr);
    }
    mVertexShaderModule = VK_NULL_HANDLE;
}

void VulkanVertexShader::VertexAttribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
    VkVertexInputAttributeDescription attribute{};
    attribute.binding = binding;
    attribute.location = location;
    attribute.format = format;
    attribute.offset = offset;

    attributes.push_back(attribute);
}

void VulkanVertexShader::VertexAttributeMatrix4f(uint32_t binding, uint32_t location)
{
    for (int i = 0; i < 4; i++)
    {
        VkVertexInputAttributeDescription attribute{};
        attribute.binding = binding;
        attribute.location = location + i;
        attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribute.offset = i * sizeof(glm::vec4);

        attributes.push_back(attribute);
    }
}

void VulkanVertexShader::VertexUniformBinding(uint32_t bindingNumber, uint32_t stride, VkVertexInputRate inputRate)
{
    VkVertexInputBindingDescription binding{};
    binding.binding = bindingNumber;
    binding.stride = stride;
    binding.inputRate = inputRate;

    bindings.push_back(binding);
}

VkPipelineVertexInputStateCreateInfo VulkanVertexShader::GetVertexInputStateInfo()
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = bindings.size();
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();

    vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    return vertexInputInfo;
}

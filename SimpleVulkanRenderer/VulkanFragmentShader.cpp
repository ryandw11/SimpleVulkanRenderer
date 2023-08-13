#include "VulkanFragmentShader.hpp"

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

VulkanFragmentShader::VulkanFragmentShader(VkDevice device, std::string name, std::string file) : mFunctionStartName(name)
{
    auto vertexShaderCode = readFile(file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = vertexShaderCode.size();
    // The api takes in a uint32_t instead of a character pointer for byte data.
    createInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderCode.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &mFragmentShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = mFragmentShaderModule;
    fragmentShaderStageInfo.pName = mFunctionStartName.c_str();

    this->fragmentShaderStageInfo = fragmentShaderStageInfo;
}

VkPipelineShaderStageCreateInfo VulkanFragmentShader::GetShaderStage()
{
    return fragmentShaderStageInfo;
}

void VulkanFragmentShader::DestroyShaderModuleIfNeeded(VkDevice device)
{
    if (mFragmentShaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device, mFragmentShaderModule, nullptr);
    }
    mFragmentShaderModule = VK_NULL_HANDLE;
}

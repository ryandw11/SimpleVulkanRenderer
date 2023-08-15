#include "VulkanDescriptorLayout.hpp"

#include <stdexcept>
#include <map>

VulkanDescriptorLayout::VulkanDescriptorLayout(VkDevice device)
	:
	mDevice(device),
    mLayout(nullptr),
    mBuiltPool(nullptr),
    mSetBuilder(nullptr)
{
}

void VulkanDescriptorLayout::UniformBufferBinding(uint32_t binding, uint32_t count, VkShaderStageFlags stageFlags)
{
    ValidateLayoutNotYetBuilt();

    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.pImmutableSamplers = nullptr;

    AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, layoutBinding);
}

void VulkanDescriptorLayout::ImageSamplerBinding(uint32_t binding, uint32_t count, VkShaderStageFlags stageFlags)
{
    ValidateLayoutNotYetBuilt();
    
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = binding;
    samplerLayoutBinding.descriptorCount = count;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = stageFlags;

    AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, samplerLayoutBinding);
}

void VulkanDescriptorLayout::GenericLayoutBinding(VkDescriptorSetLayoutBinding binding, VkDescriptorType type)
{
    ValidateLayoutNotYetBuilt();
    AddBinding(type, binding);
}

VkDescriptorSetLayout VulkanDescriptorLayout::BuildLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    for (const auto& bindingInfo : mLayoutBindings)
    {
        layoutBindings.push_back(bindingInfo.binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();
    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    return mLayout;
}

bool VulkanDescriptorLayout::IsBuilt()
{
    return mLayout != VK_NULL_HANDLE;
}

VkDescriptorPool VulkanDescriptorLayout::CreateDescriptorPool(uint32_t descriptorSetCount)
{
    if (mBuiltPool != VK_NULL_HANDLE)
    {
        return mBuiltPool;
    }

    std::map<VkDescriptorType, uint32_t> poolTypes;

    for (const auto& bindingInfo : mLayoutBindings)
    {
        poolTypes[bindingInfo.type] = poolTypes[bindingInfo.type] + 1;
    }
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto& poolType : poolTypes)
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type = poolType.first;
        poolSize.descriptorCount = poolType.second * descriptorSetCount;
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = descriptorSetCount;

    if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mBuiltPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    mSetBuilder = std::make_shared<VulkanDescriptorSetBuilder>(mDevice, mBuiltPool, mLayout, descriptorSetCount);

    return mBuiltPool;
}

std::shared_ptr<VulkanDescriptorSetBuilder> VulkanDescriptorLayout::DescriptorSetBuilder()
{
    ValidateNonNull(mSetBuilder, "Create the descriptor pool before accessing the builder!");
    return mSetBuilder;
}

void VulkanDescriptorLayout::ValidateLayoutNotYetBuilt()
{
    if (mLayout != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempted to add layout binding after it was already built!");
    }
}

void VulkanDescriptorLayout::AddBinding(VkDescriptorType type, VkDescriptorSetLayoutBinding binding)
{
    mLayoutBindings.push_back({ type, binding });
}

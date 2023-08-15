#include "VulkanDescriptorSetBuilder.hpp"

#include <stdexcept>

VulkanDescriptorSetBuilder::VulkanDescriptorSetBuilder(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t setCount)
	:
	mDevice(device)
{
	std::vector<VkDescriptorSetLayout> layouts(setCount, layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = layouts.data();

    mDescriptorSets.resize(setCount);
    if (vkAllocateDescriptorSets(device, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
}

void VulkanDescriptorSetBuilder::DescribeBuffer(uint32_t binding, uint32_t arrayElement, VulkanFrameBuffer buffer, VkDeviceSize range)
{
    DescriptorSetInfo setInfo;

    FrameDescriptorBufferInfo bufferInfo{ buffer, range };

    setInfo.DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setInfo.BufferInfo = bufferInfo;
    setInfo.IsBuffer = true;
    setInfo.DstBinding = binding;
    setInfo.DstArrayElement = arrayElement;

    mSetInfos.push_back(setInfo);
}

void VulkanDescriptorSetBuilder::DescribeImageSample(uint32_t binding, uint32_t arrayElement, VulkanFrameImageView imageView, VulkanFrameSampler sampler)
{
    DescriptorSetInfo setInfo;

    FrameDescriptorImageInfo imageInfo {imageView, sampler};

    setInfo.DescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setInfo.ImageInfo = imageInfo;
    setInfo.IsBuffer = false;
    setInfo.DstBinding = binding;
    setInfo.DstArrayElement = arrayElement;

    mSetInfos.push_back(setInfo);
}

std::vector<VkDescriptorSet> VulkanDescriptorSetBuilder::UpdateDescriptorSets()
{
    for (size_t i = 0; i < mDescriptorSets.size(); i++)
    {
        // Prevent buffer and image infos from being out of scoped before the update occurs.
        std::vector<std::shared_ptr<VkDescriptorBufferInfo>> bufferSink;
        std::vector<std::shared_ptr<VkDescriptorImageInfo>> imageSink;

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        for (auto setInfo : mSetInfos)
        {
            VkWriteDescriptorSet writer{};
            writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writer.dstSet = mDescriptorSets[i];
            writer.dstBinding = setInfo.DstBinding;
            writer.dstArrayElement = setInfo.DstArrayElement;
            writer.descriptorType = setInfo.DescriptorType;
            writer.descriptorCount = 1;
            if (setInfo.IsBuffer)
            {
                auto bufferInfo = std::make_shared<VkDescriptorBufferInfo>();
                bufferInfo->buffer = setInfo.BufferInfo->FrameBuffer[i];
                bufferInfo->offset = 0;
                bufferInfo->range = setInfo.BufferInfo->Range;
                writer.pBufferInfo = bufferInfo.get();
                writer.pImageInfo = nullptr;
                bufferSink.push_back(bufferInfo);
            }
            else
            {
                auto imageInfo = std::make_shared<VkDescriptorImageInfo>();
                imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo->imageView = setInfo.ImageInfo->FrameImageView[i];
                imageInfo->sampler = setInfo.ImageInfo->FrameSampler[i];
                writer.pBufferInfo = nullptr;
                writer.pImageInfo = imageInfo.get();
                imageSink.push_back(imageInfo);
            }
            writer.pTexelBufferView = nullptr;

            descriptorWrites.push_back(writer);
        }
        vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return mDescriptorSets;
}

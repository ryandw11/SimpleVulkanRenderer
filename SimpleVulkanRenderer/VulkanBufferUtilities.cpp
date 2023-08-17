#include "VulkanBufferUtilities.hpp"
#include "VulkanCommandBuffer.hpp"

namespace
{
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }
}

VulkanBufferUtilities::VulkanBufferUtilities(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool defaultCommandPool, VkQueue defaultGraphicsQueue)
	:
	mPhysicalDevice(physicalDevice),
	mDevice(device),
    mDefaultCommandPool(defaultCommandPool),
    mDefaultGraphicsQueue(defaultGraphicsQueue)
{
}

void VulkanBufferUtilities::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& outBuffer, VkDeviceMemory& outBufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &outBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice, outBuffer, &memRequirements);

    // Allocate the memory.
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &outBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    // Associate the created memory with the buffer.
    vkBindBufferMemory(mDevice, outBuffer, outBufferMemory, 0);
}

VulkanBuffer VulkanBufferUtilities::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VulkanBuffer buffer;
    CreateBuffer(size, usage, properties, buffer, buffer);

    return buffer;
}

void VulkanBufferUtilities::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue)
{
    
    VkCommandPool usedCommandPool = mDefaultCommandPool;
    if (commandPool != VK_NULL_HANDLE)
    {
        usedCommandPool = commandPool;
    }

    VkQueue usedQueue = mDefaultGraphicsQueue;
    if (queue != VK_NULL_HANDLE)
    {
        usedQueue = queue;
    }

    auto commandBuffer = CreateSingleUseCommandBuffer(mDevice, usedCommandPool);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    commandBuffer->CopyBuffer(srcBuffer, dstBuffer, copyRegion);
    commandBuffer->SubmitSingleUseCommand(mDevice, usedQueue);
}

void VulkanBufferUtilities::MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize bufferSize, VkMemoryMapFlags flags, void** data)
{
    vkMapMemory(mDevice, memory, offset, bufferSize, flags, data);
}

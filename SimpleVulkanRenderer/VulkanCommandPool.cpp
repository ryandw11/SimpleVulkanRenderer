#include "VulkanCommandPool.hpp"
#include "VulkanRendererTypes.hpp"
#include "VulkanCommandBuffer.hpp"

#include <stdexcept>

VulkanCommandPool::VulkanCommandPool(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, std::string identifier, std::optional<VulkanQueue> vulkanQueue)
    :
    mIdentifier(identifier),
    mOwningThread(std::this_thread::get_id())
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(surface, physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // The graphics family queue index.
    poolInfo.queueFamilyIndex = vulkanQueue ? vulkanQueue->QueueFamily : queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

std::shared_ptr<VulkanCommandBuffer> VulkanCommandPool::CreateCommandBuffer(VkDevice device)
{
    auto commandBuffer = std::make_shared<VulkanCommandBuffer>(device, mCommandPool, mOwningThread);
    mCommandBuffers.push_back(commandBuffer);

    return commandBuffer;
}

void VulkanCommandPool::FreeCommandBuffers(VkDevice device)
{
    for (auto commandBuffer : mCommandBuffers)
    {
        commandBuffer->FreeCommandBuffer(device);
    }
    mCommandBuffers.clear();
}

void VulkanCommandPool::DestroyCommandPool(VkDevice device)
{
    vkDestroyCommandPool(device, mCommandPool, nullptr);
}

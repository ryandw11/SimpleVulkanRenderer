#include "VulkanBuffer.hpp"

VulkanBuffer::VulkanBuffer()
	:
	mInternalBuffer(VK_NULL_HANDLE),
	mInternalMemory(VK_NULL_HANDLE)
{
}

VulkanBuffer::VulkanBuffer(VkBuffer buffer, VkDeviceMemory deviceMemory)
	:
	mInternalBuffer(buffer),
	mInternalMemory(deviceMemory)
{
}

void VulkanBuffer::DestoryBuffer(VkDevice device)
{
	vkDestroyBuffer(device, mInternalBuffer, nullptr);
	vkFreeMemory(device, mInternalMemory, nullptr);
	mInternalBuffer = nullptr;
	mInternalMemory = nullptr;
}

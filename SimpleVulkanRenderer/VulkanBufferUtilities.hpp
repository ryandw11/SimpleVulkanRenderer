#pragma once
#ifndef VULKAN_BUFFER_UTILITIES_H
#define VULKAN_BUFFER_UTILITIES_H

#include "VulkanIncludes.hpp"
#include "VulkanBuffer.hpp"

class VulkanBufferUtilities
{
public:
	VulkanBufferUtilities(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool defaultCommandPool, VkQueue defaultGraphicsQueue);
	
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& outBuffer, VkDeviceMemory& bufferMemory);
	VulkanBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool = nullptr, VkQueue queue = nullptr);

	// ---------------------------------------------------
	// Specific Buffer Creation
	// ---------------------------------------------------
	template<typename T>
	void CreateVertexBuffer(std::vector<T> vertexData, VkBuffer& outVertexBuffer, VkDeviceMemory& outVertexBufferMemory, VkCommandPool commandPool = nullptr, VkQueue queue = nullptr)
	{
		VkDeviceSize bufferSize = sizeof(vertexData[0]) * vertexData.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy the vertex data to the buffer.
		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertexData.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outVertexBuffer, outVertexBufferMemory);

		CopyBuffer(stagingBuffer, outVertexBuffer, bufferSize, commandPool, queue);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	template<typename T>
	VulkanBuffer CreateVertexBuffer(std::vector<T> vertexData, VkCommandPool commandPool = nullptr, VkQueue queue = nullptr)
	{
		VulkanBuffer buffer;
		CreateVertexBuffer(vertexData, buffer, buffer, commandPool, queue);
		return buffer;
	}

	template<typename T>
	void CreateIndexBuffer(std::vector<T> indexData, VkBuffer& outIndexBuffer, VkDeviceMemory& outIndexBufferMemory, VkCommandPool commandPool = nullptr, VkQueue queue = nullptr)
	{
		VkDeviceSize bufferSize = sizeof(indexData[0]) * indexData.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy the index data to the buffer.
		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indexData.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outIndexBuffer, outIndexBufferMemory);

		CopyBuffer(stagingBuffer, outIndexBuffer, bufferSize, commandPool, queue);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	template<typename T>
	VulkanBuffer CreateIndexBuffer(std::vector<T> indexData, VkCommandPool commandPool = nullptr, VkQueue queue = nullptr)
	{
		VulkanBuffer buffer;
		CreateIndexBuffer(indexData, buffer, buffer, commandPool, queue);
		return buffer;
	}

	// ---------------------------------------------------
	// Buffer Memory Mapping
	// ---------------------------------------------------

	/// <summary>
	/// Creates a persistent mapping to a buffer (actually it's device memory).
	/// </summary>
	/// <param name="memory">The memory of the buffer.</param>
	/// <param name="offset">The offset to map to.</param>
	/// <param name="bufferSize">The size of the buffer.</param>
	/// <param name="flags">The flags.</param>
	/// <param name="data">The wild pointer which you can memcpy your data to.</param>
	void MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize bufferSize, VkMemoryMapFlags flags, void** data);

private:
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkCommandPool mDefaultCommandPool;
	VkQueue mDefaultGraphicsQueue;
};

#endif

#pragma once
#ifndef VULKAN_COMMAND_BUFFER_H
#define VULKAN_COMMAND_BUFFER_H

#include <thread>

#include "VulkanIncludes.hpp"

class VulkanCommandBuffer
{
public:
	VulkanCommandBuffer(VkDevice device, VkCommandPool parentPool, std::thread::id parentPoolId);

	// ---------------------------------------------------
	// Starts
	// ---------------------------------------------------
	void StartCommandRecording();
	void StartSingleUseCommandRecording();
	void StartRenderPass(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }, VkClearDepthStencilValue depthStencil = { 1.0f, 0 });
	// ---------------------------------------------------
	// Binds
	// ---------------------------------------------------
	void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
	void BindVertexBuffer(VkBuffer buffer, VkDeviceSize offset = 0, uint32_t firstBinding = 0);
	void BindVertexBuffers(VkBuffer buffers[], VkDeviceSize offsets[], uint32_t bufferCount, uint32_t firstBinding = 0);
	void BindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32);
	void BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
	// ---------------------------------------------------
	// Draw
	// ---------------------------------------------------
	void DrawIndexed(uint32_t indexSize, uint32_t instanceCount = 1, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstace = 0);
	// ---------------------------------------------------
	// State Setting
	// ---------------------------------------------------
	void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
	void SetScissor(VkOffset2D offset, VkExtent2D extent);
	void SetViewportScissor(VkExtent2D swapChainExtent);
	void Reset(VkCommandBufferResetFlags resetFlags = 0);
	// ---------------------------------------------------
	// Memory Copying
	// ---------------------------------------------------
	void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkBufferCopy copyRegion);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	// ---------------------------------------------------
	// General Memory Changes
	// ---------------------------------------------------
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	// ---------------------------------------------------
	// Ends
	// ---------------------------------------------------
	void EndRenderPass();
	void EndCommandRecording();
	// ---------------------------------------------------
	// Submit
	// ---------------------------------------------------
	void SubmitSingleUseCommand(VkDevice device, VkQueue queue);
	void Submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence);
	void Submit(VkQueue queue, VkSubmitInfo submitInfo, VkFence fence);
	// ---------------------------------------------------
	// Freeing
	// ---------------------------------------------------
	void FreeCommandBuffer(VkDevice device);

	operator VkCommandBuffer()
	{
		return mCommandBuffer;
	}
private:
	std::thread::id mParentPoolThread;

	VkCommandBuffer mCommandBuffer;
	VkCommandPool mParentPool;
};

/// <summary>
/// Create a single use command buffer that is ready for recording.
/// 
/// #StartSingleUseCommandRecording() is already called.
/// </summary>
/// <param name="device">The device.</param>
/// <param name="commandPool">The pool for the command.</param>
/// <returns>The single use command buffer.</returns>
static std::shared_ptr<VulkanCommandBuffer> CreateSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool)
{
	auto commandBuffer = std::make_shared<VulkanCommandBuffer>(device, commandPool, std::this_thread::get_id());
	commandBuffer->StartSingleUseCommandRecording();

	return commandBuffer;
}

#endif
#include "VulkanCommandBuffer.hpp"

#include <array>
#include <stdexcept>

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool parentPool, std::thread::id parentPoolId)
	:
	mParentPoolThread(parentPoolId),
	mParentPool(parentPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = parentPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &allocInfo, &mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer!");
	}
}

void VulkanCommandBuffer::StartCommandRecording()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
}

void VulkanCommandBuffer::StartSingleUseCommandRecording()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(mCommandBuffer, &beginInfo);
}

void VulkanCommandBuffer::StartRenderPass(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkClearColorValue clearColor, VkClearDepthStencilValue depthStencil)
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 2> clearValues{};

	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = depthStencil;

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint)
{
	vkCmdBindPipeline(mCommandBuffer, bindPoint, pipeline);
}

void VulkanCommandBuffer::BindVertexBuffer(VkBuffer buffer, VkDeviceSize offset, uint32_t firstBinding)
{
	VkBuffer vertexBuffers[] = { buffer };
	VkDeviceSize offsets[] = { 0 };
	BindVertexBuffers(vertexBuffers, offsets, 1, firstBinding);
}

void VulkanCommandBuffer::BindVertexBuffers(VkBuffer buffers[], VkDeviceSize offsets[], uint32_t bufferCount, uint32_t firstBinding)
{
	vkCmdBindVertexBuffers(mCommandBuffer, firstBinding, bufferCount, buffers, offsets);
}

void VulkanCommandBuffer::BindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(mCommandBuffer, indexBuffer, offset, indexType);
}

void VulkanCommandBuffer::BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet, VkPipelineBindPoint bindPoint)
{
	vkCmdBindDescriptorSets(mCommandBuffer, bindPoint, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}

void VulkanCommandBuffer::DrawIndexed(uint32_t indexSize, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstace)
{
	vkCmdDrawIndexed(mCommandBuffer, indexSize, instanceCount, firstIndex, vertexOffset, firstInstace);
}

void VulkanCommandBuffer::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	VkViewport viewport{};
	viewport.x = x;
	viewport.y = y;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::SetScissor(VkOffset2D offset, VkExtent2D extent)
{
	VkRect2D scissor{};
	scissor.offset = offset;
	scissor.extent = extent;
	vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::SetViewportScissor(VkExtent2D swapChainExtent)
{
	SetViewport(0, 0, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0, 1);
	SetScissor({ 0, 0 }, swapChainExtent);
}

void VulkanCommandBuffer::Reset(VkCommandBufferResetFlags resetFlags)
{
	vkResetCommandBuffer(mCommandBuffer, resetFlags);
}

void VulkanCommandBuffer::CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkBufferCopy copyRegion)
{
	vkCmdCopyBuffer(mCommandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);
}

void VulkanCommandBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		mCommandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
}

void VulkanCommandBuffer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		mCommandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

void VulkanCommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(mCommandBuffer);
}

void VulkanCommandBuffer::EndCommandRecording()
{
	if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to finish recording command buffer!");
	}
}

void VulkanCommandBuffer::SubmitSingleUseCommand(VkDevice device, VkQueue queue)
{
	EndCommandRecording();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, mParentPool, 1, &mCommandBuffer);
}

void VulkanCommandBuffer::Submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
{
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = waitSemaphore != nullptr ? 1 : 0;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphore != nullptr ? 1 : 0;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer!");
	}
}

void VulkanCommandBuffer::Submit(VkQueue queue, VkSubmitInfo submitInfo, VkFence fence)
{
	// Set the type to be certain.
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer!");
	}
}

void VulkanCommandBuffer::FreeCommandBuffer(VkDevice device)
{
	vkFreeCommandBuffers(device, mParentPool, 1, &mCommandBuffer);
}

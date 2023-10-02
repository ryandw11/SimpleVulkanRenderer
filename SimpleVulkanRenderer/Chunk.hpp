#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VulkanIncludes.hpp"
#include "VulkanRendererTypes.hpp"

#include "VulkanDescriptorSetBuilder.hpp"
#include "VulkanDescriptorLayout.hpp"
#include "VulkanRenderer.hpp"

#include <atomic>

class Chunk
{
public:
	Chunk();
	Chunk(glm::vec3 location);
	void GenerateChunk(Ptr(VulkanBufferUtilities) bufferUtils, Ptr(VulkanCommandPool) commandPool, VulkanQueue queue);

	VulkanBuffer& VertexBuffer();
	VulkanBuffer& IndexBuffer();
	VulkanMappedBuffer& ModelBuffer();

	size_t IndiciesSize();
	glm::vec3 Location();

	std::atomic_bool& FinishedGenerating();
private:
	glm::vec3 mLocation;

	std::vector<Vertex> mVertices;
	VulkanBuffer mVertexBuffer;
	std::vector<uint32_t> mIndices;
	VulkanBuffer mIndexBuffer;

	VulkanMappedBuffer mModelBuffer;
	std::atomic_bool mFinishedGenerating;
};

#endif
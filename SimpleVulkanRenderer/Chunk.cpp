#include "Chunk.hpp"
#include "GreedyMesh.hpp"
#include "DemoConsts.hpp"


Chunk::Chunk()
    :
    mFinishedGenerating(false),
    mLocation({0, 0, 0})
{
}

Chunk::Chunk(glm::vec3 location)
	:
	mFinishedGenerating(false),
    mLocation(location)
{
}

void Chunk::GenerateChunk(Ptr(VulkanBufferUtilities) bufferUtils, Ptr(VulkanCommandPool) commandPool, VulkanQueue queue)
{
    int*** chunkArray = new int** [CHUNK_VOXEL_COUNT];
    for (int x = 0; x < CHUNK_VOXEL_COUNT; x++) {
        chunkArray[x] = new int* [CHUNK_VOXEL_COUNT];
        for (int y = 0; y < CHUNK_VOXEL_COUNT; y++) {
            chunkArray[x][y] = new int[CHUNK_VOXEL_COUNT];
            for (int z = 0; z < CHUNK_VOXEL_COUNT; z++) {
                chunkArray[x][y][z] = rand() % 2;
            }
        }
    }

    AlgorithmOutput output = greedyMeshAlgorithm(chunkArray, CHUNK_VOXEL_COUNT, -1);

    mVertices.reserve(mVertices.size() + output.verticies.size());
    mVertices.insert(mVertices.end(), output.verticies.begin(), output.verticies.end());

    mIndices.reserve(mIndices.size() + output.indicies.size());
    mIndices.insert(mIndices.end(), output.indicies.begin(), output.indicies.end());

    // Vertex Buffer
    mVertexBuffer = bufferUtils->CreateVertexBuffer(mVertices, commandPool->CommandPool(), queue.queue);

    // Index Buffer
    bufferUtils->CreateIndexBuffer(mIndices, mIndexBuffer, mIndexBuffer, commandPool->CommandPool(), queue.queue);

    // Model Buffer
    bufferUtils->CreateBuffer(sizeof(glm::mat4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mModelBuffer, mModelBuffer);
    bufferUtils->MapMemory(mModelBuffer, 0, sizeof(glm::mat4), 0, mModelBuffer.DirectMappedMemory());

    mFinishedGenerating = true;
}

VulkanBuffer& Chunk::VertexBuffer()
{
    return mVertexBuffer;
}

VulkanBuffer& Chunk::IndexBuffer()
{
    return mIndexBuffer;
}

VulkanMappedBuffer& Chunk::ModelBuffer()
{
    return mModelBuffer;
}

size_t Chunk::IndiciesSize()
{
    return mIndices.size();
}

glm::vec3 Chunk::Location()
{
    return mLocation;
}

std::atomic_bool& Chunk::FinishedGenerating()
{
    return mFinishedGenerating;
}

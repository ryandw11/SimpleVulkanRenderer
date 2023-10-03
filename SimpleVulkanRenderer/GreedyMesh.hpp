#pragma once
#ifndef GREEDY_MESH_H
#define GREEDY_MESH_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <array>
#include <unordered_set>
#include <string.h>
#include <queue>

#include "Queue.h"
#include "3DArray.h"

#include "VulkanRendererTypes.hpp"

/**

    The output of the voxel greedy mesh aglorithm.

*/
struct AlgorithmOutput {
    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;
};

// Macros to define 1/3 and 2/3
#define ONE_THIRD (1/3)
#define TWO_THIRD (2/3)

constexpr auto GREEN = glm::vec3(0, 0.75, 0);
constexpr auto BROWN = glm::vec3(0.588, 0.31, 0.008);

/**
    Add the front face of a voxel to the output. O(1)

    @param pos The position of the voxel
    @param output The output to add to.
    @param i The current indices value.
*/
void getFront(glm::vec3 position, AlgorithmOutput& output, int i) {
    Vertex v1;
    v1.pos = glm::vec3(-0.5f + position.x, 0.5f + position.y, 0.5f + position.z);
    v1.color = BROWN;
    v1.texCoord = glm::vec2(1, 1);
    Vertex v2;
    v2.pos = glm::vec3(-0.5f + position.x, -0.5f + position.y, 0.5f + position.z);
    v2.color = BROWN;
    v2.texCoord = glm::vec2(1, 1);
    Vertex v3;
    v3.pos = glm::vec3(0.5f + position.x, -0.5f + position.y, 0.5f + position.z);
    v3.color = BROWN;
    v3.texCoord = glm::vec2(1, 1);
    Vertex v4;
    v4.pos = glm::vec3(0.5f + position.x, 0.5f + position.y, 0.5f + position.z);
    v4.color = BROWN;
    v4.texCoord = glm::vec2(1, 1);

    output.verticies.push_back(v1);
    output.verticies.push_back(v2);
    output.verticies.push_back(v3);
    output.verticies.push_back(v4);
    output.indicies.push_back(i);
    output.indicies.push_back(i + 1);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 3);
    output.indicies.push_back(i);
}

/**
    Add the back face of a voxel to the output. O(1)

    @param pos The position of the voxel
    @param output The output to add to.
    @param i The current indices value.
*/
void getBack(glm::vec3 pos, AlgorithmOutput& output, int i) {
    Vertex v1;
    v1.pos = glm::vec3(-0.5f + pos.x, 0.5f + pos.y, -0.5f + pos.z);
    v1.color = BROWN;
    v1.texCoord = glm::vec2(0, 0);
    Vertex v2;
    v2.pos = glm::vec3(-0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z);
    v2.color = BROWN;
    v2.texCoord = glm::vec2(1, 1);
    Vertex v3;
    v3.pos = glm::vec3(0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z);
    v3.color = BROWN;
    v3.texCoord = glm::vec2(0, 1);
    Vertex v4;
    v4.pos = glm::vec3(0.5f + pos.x, 0.5f + pos.y, -0.5f + pos.z);
    v4.color = BROWN;
    v4.texCoord = glm::vec2(1, 0);

    output.verticies.push_back(v1);
    output.verticies.push_back(v2);
    output.verticies.push_back(v3);
    output.verticies.push_back(v4);
    output.indicies.push_back(i);
    output.indicies.push_back(i + 3);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 1);
    output.indicies.push_back(i);
}

/**
    Add the top face of a voxel to the output. O(1)

    @param pos The position of the voxel
    @param output The output to add to.
    @param i The current indices value.
*/
void getTop(glm::vec3 pos, AlgorithmOutput& output, int i) {
    Vertex v1;
    v1.pos = glm::vec3(-0.5f + pos.x, 0.5f + pos.y, -0.5f + pos.z);
    v1.color = GREEN;
    v1.texCoord = glm::vec2(0, 0);
    Vertex v2;
    v2.pos = glm::vec3(-0.5f + pos.x, 0.5f + pos.y, 0.5f + pos.z);
    v2.color = GREEN;
    v2.texCoord = glm::vec2(1, 1);
    Vertex v3;
    v3.pos = glm::vec3(0.5f + pos.x, 0.5f + pos.y, 0.5f + pos.z);
    v3.color = GREEN;
    v3.texCoord = glm::vec2(1, 0);
    Vertex v4;
    v4.pos = glm::vec3(0.5f + pos.x, 0.5f + pos.y, -0.5f + pos.z);
    v4.color = GREEN;
    v4.texCoord = glm::vec2(0, 1);

    output.verticies.push_back(v1);
    output.verticies.push_back(v2);
    output.verticies.push_back(v3);
    output.verticies.push_back(v4);
    output.indicies.push_back(i);
    output.indicies.push_back(i + 1);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 3);
    output.indicies.push_back(i);
}

/**
    Add the bottom face of a voxel to the output. O(1)

    @param pos The position of the voxel
    @param output The output to add to.
    @param i The current indices value.
*/
void getBottom(glm::vec3 pos, AlgorithmOutput& output, int i) {
    Vertex v1;
    v1.pos = glm::vec3(-0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z);
    v1.color = BROWN;
    v1.texCoord = glm::vec2(0, 0);
    Vertex v2;
    v2.pos = glm::vec3(-0.5f + pos.x, -0.5f + pos.y, 0.5f + pos.z);
    v2.color = BROWN;
    v2.texCoord = glm::vec2(1, 0);
    Vertex v3;
    v3.pos = glm::vec3(0.5f + pos.x, -0.5f + pos.y, 0.5f + pos.z);
    v3.color = BROWN;
    v3.texCoord = glm::vec2(0, 1);
    Vertex v4;
    v4.pos = glm::vec3(0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z);
    v4.color = BROWN;
    v4.texCoord = glm::vec2(1, 1);

    output.verticies.push_back(v1);
    output.verticies.push_back(v2);
    output.verticies.push_back(v3);
    output.verticies.push_back(v4);
    output.indicies.push_back(i);
    output.indicies.push_back(i + 3);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 1);
    output.indicies.push_back(i);
}

/**
    Add the right face of a voxel to the output. O(1)

    @param pos The position of the voxel
    @param output The output to add to.
    @param i The current indices value.
*/
void getRight(glm::vec3 pos, AlgorithmOutput& output, int i) {
    Vertex v1;
    v1.pos = glm::vec3(0.5f + pos.x, 0.5f + pos.y, 0.5f + pos.z);
    v1.color = BROWN;
    v1.texCoord = glm::vec2(0, 0);
    Vertex v2;
    v2.pos = glm::vec3(0.5f + pos.x, -0.5f + pos.y, 0.5f + pos.z);
    v2.color = BROWN;
    v2.texCoord = glm::vec2(0, 1);
    Vertex v3;
    v3.pos = glm::vec3(0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z);
    v3.color = BROWN;
    v3.texCoord = glm::vec2(1, 0);
    Vertex v4;
    v4.pos = glm::vec3(0.5f + pos.x, 0.5f + pos.y, -0.5f + pos.z);
    v4.color = BROWN;
    v4.texCoord = glm::vec2(1, 1);

    output.verticies.push_back(v1);
    output.verticies.push_back(v2);
    output.verticies.push_back(v3);
    output.verticies.push_back(v4);
    output.indicies.push_back(i); 
    output.indicies.push_back(i + 1); 
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 3);
    output.indicies.push_back(i);
}

/**
    Add the left face of a voxel to the output. O(1)

    @param pos The position of the voxel
    @param output The output to add to.
    @param i The current indices value.
*/
void getLeft(glm::vec3 pos, AlgorithmOutput& output, int i) {
    Vertex v1;
    v1.pos = glm::vec3(-0.5f + pos.x, 0.5f + pos.y, -0.5f + pos.z);
    v1.color = BROWN;
    v1.texCoord = glm::vec2(0, 0);
    Vertex v2;
    v2.pos = glm::vec3(-0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z);
    v2.color = BROWN;
    v2.texCoord = glm::vec2(0, 1);
    Vertex v3;
    v3.pos = glm::vec3(-0.5f + pos.x, -0.5f + pos.y, 0.5f + pos.z);
    v3.color = BROWN;
    v3.texCoord = glm::vec2(1, 0);
    Vertex v4;
    v4.pos = glm::vec3(-0.5f + pos.x, 0.5f + pos.y, 0.5f + pos.z);
    v4.color = BROWN;
    v4.texCoord = glm::vec2(1, 1);

    output.verticies.push_back(v1);
    output.verticies.push_back(v2);
    output.verticies.push_back(v3);
    output.verticies.push_back(v4);
    output.indicies.push_back(i);
    output.indicies.push_back(i + 1);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 2);
    output.indicies.push_back(i + 3);
    output.indicies.push_back(i);
}

bool checkBounds(glm::vec3 vec, int chunkSize);
int getChunkData(int*** chunkArray, int realChunkSize, glm::vec3 vec);

AlgorithmOutput greedyMeshAlgorithm(int*** chunkArray, int chunkSize, int voxelCount) {
    int totalIterations = 0;

    AlgorithmOutput output;
    if (voxelCount == 0) return output;
    // Edge Case: If the entire chunk is full.
    if (voxelCount == chunkSize * chunkSize * chunkSize) {
        int i = 0;
        for (int x = 0; x < chunkSize; x++) {
            for (int y = 0; y < chunkSize; y++) {
                // O(1)
                getBack(glm::vec3(x, y, 0), output, i);
                i+=4;
            }
        }
        for (int x = 0; x < chunkSize; x++) {
            for (int y = 0; y < chunkSize; y++) {
                // O(1)
                getFront(glm::vec3(x, y, chunkSize - 1), output, i);
                i += 4;
            }
        }
        for (int z = 0; z < chunkSize; z++) {
            for (int y = 0; y < chunkSize; y++) {
                getLeft(glm::vec3(0, y, z), output, i);
                i += 4;
            }
        }
        for (int z = 0; z < chunkSize; z++) {
            for (int y = 0; y < chunkSize; y++) {
                getRight(glm::vec3(chunkSize - 1, y, z), output, i);
                i += 4;
            }
        }
        for (int x = 0; x < chunkSize; x++) {
            for (int z = 0; z < chunkSize; z++) {
                getTop(glm::vec3(x, chunkSize-1, z), output, i);
                i += 4;
            }
        }
        for (int x = 0; x < chunkSize; x++) {
            for (int z = 0; z < chunkSize; z++) {
                getBottom(glm::vec3(x, 0, z), output, i);
                i += 4;
            }
        }
        return output;
    }
    int i = 0;
    int nChunkSize = chunkSize + 2;
    // Initalize pi 3d array.
    ThreeDArray<int> pi(nChunkSize);
    pi.zeroOut(); // Zero out the 3D array O(n) (Faster than manual for loop in practice)
    
    std::queue<glm::vec3> voxelsToVisit;


    glm::vec3 firstEdge = glm::vec3(0, 0, 0);
    pi.set(firstEdge, 1); // O(1)
    voxelsToVisit.push(firstEdge); // Add to the queue. O(1)
    // Worst case: O((n+2)^3)
    while (!voxelsToVisit.empty()) {
        totalIterations++;
        glm::vec3 voxelToProccess = voxelsToVisit.front(); // Get the front of the queue. O(1)
        voxelsToVisit.pop(); // Pop from the queue. O(1)

        glm::vec3 checkVoxel = voxelToProccess + glm::vec3(0, 0, 1);
        // Check Front
        if (checkBounds(checkVoxel, nChunkSize)
            && pi.at(checkVoxel) == 0) {
            if (getChunkData(chunkArray, chunkSize, checkVoxel) == 1) {
                getBack(checkVoxel, output, i);
                i += 4;
            }
            else {
                voxelsToVisit.push(checkVoxel);
                pi.set(checkVoxel, 1);
            }

        }
        // Check Back
        checkVoxel = voxelToProccess + glm::vec3(0, 0, -1);
        if (checkBounds(checkVoxel, nChunkSize)
            && pi.at(checkVoxel) == 0) {
            if (getChunkData(chunkArray, chunkSize, checkVoxel) == 1) {
                getFront(checkVoxel, output, i);
                i += 4;
            }
            else {
                voxelsToVisit.push(checkVoxel);
                pi.set(checkVoxel, 1);
            }
        }
        // Check Top
        checkVoxel = voxelToProccess + glm::vec3(0, 1, 0);
        if (checkBounds(checkVoxel, nChunkSize)
            && pi.at(checkVoxel) == 0) {
            if (getChunkData(chunkArray, chunkSize, checkVoxel) == 1) {
                getBottom(checkVoxel, output, i);
                i += 4;
            }
            else {
                voxelsToVisit.push(checkVoxel);
                pi.set(checkVoxel, 1);
            }
        }
        // Check Bottom
        checkVoxel = voxelToProccess + glm::vec3(0, -1, 0);
        if (checkBounds(checkVoxel, nChunkSize)
            && pi.at(checkVoxel) == 0) {
            if (getChunkData(chunkArray, chunkSize, checkVoxel) == 1) {
                getTop(checkVoxel, output, i);
                i += 4;
            }
            else {
                voxelsToVisit.push(checkVoxel);
                pi.set(checkVoxel, 1);
            }

        }
        // Check Right
        checkVoxel = voxelToProccess + glm::vec3(1, 0, 0);
        if (checkBounds(checkVoxel, nChunkSize)
            && pi.at(checkVoxel) == 0) {
            if (getChunkData(chunkArray, chunkSize, checkVoxel) == 1) {
                getLeft(checkVoxel, output, i);
                i += 4;
            }
            else {
                voxelsToVisit.push(checkVoxel);
                pi.set(checkVoxel, 1);
            }

        }
        // Check Left
        checkVoxel = voxelToProccess + glm::vec3(-1, 0, 0);
        if (checkBounds(checkVoxel, nChunkSize)
            && pi.at(checkVoxel) == 0) {
            if (getChunkData(chunkArray, chunkSize, checkVoxel) == 1) {
                getRight(checkVoxel, output, i);
                i += 4;
            }
            else {
                voxelsToVisit.push(checkVoxel);
                pi.set(checkVoxel, 1);
            }

        }
    }
    std::cout << "Voxel Greedy Mesh Algorithm completed in: " << totalIterations << " iterations." << std::endl;
    return output;
}

/**
    Get the chunk data from the chunk array. O(1)
*/
int getChunkData(int*** chunkArray, int realChunkSize, glm::vec3 vec) {
    if (vec.x-1 < 0 || vec.x-1 >= realChunkSize) return 0;
    if (vec.y-1 < 0 || vec.y-1 >= realChunkSize) return 0;
    if (vec.z-1 < 0 || vec.z-1 >= realChunkSize) return 0;
    return chunkArray[(int)vec.x-1][(int)vec.y-1][(int)vec.z-1];
}

/**
    Check if vector is within the voxel chunk borders. O(1)
*/
bool checkBounds(glm::vec3 vec, int chunkSize) {
    if (vec.x < 0 || vec.x >= chunkSize) return false;
    if (vec.y < 0 || vec.y >= chunkSize) return false;
    if (vec.z < 0 || vec.z >= chunkSize) return false;
    return true;
}


#endif
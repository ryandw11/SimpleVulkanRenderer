#pragma once

#ifndef VULKAN_RENDERER_TYPES
#define VULKAN_RENDERER_TYPES

#include "VulkanIncludes.hpp"

#include <string>
#include <optional>
#include <array>

struct VulkanInstanceInfo
{
	std::string ApplicationName;
	std::uint32_t ApplicationVersion;
};

// The Queue families.
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

static QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // Find a queue family that supports QUEUE GRAPHICS BIT.

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        // Find one that supports presentation. (Can be different than the graphics queue)
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        // Check if the int is populated.
        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

/**

    The struct that stores information about verticies.

*/
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    // Setup the binding descriptions.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    //Setup Attribute Descriptions.
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        // Vertex Attribute Desciption
        // Which binding the per-vertex data comes.
        attributeDescriptions[0].binding = 0;
        // The location directive of the input in the vertex shader.
        attributeDescriptions[0].location = 0;
        // Describes the type of data for the attribute. These use color formats. See the list here: https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        // The number of bytes since the start of the per-vertex data to read from.
        // That is automatically calculated using the offsetof macro.
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // Color Attribute Description.
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        /*// Texture Coord Attribute Description.
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);*/

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

/**
    A hash function for the Vertex. (This is not used by the algorithm)
*/
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

#endif
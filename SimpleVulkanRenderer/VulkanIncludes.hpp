#ifndef VULKAN_RENDERER_INCLUDES
#define VULKAN_RENDERER_INCLUDES

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
// Fix alignment with uniforms
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// Configure GLM to use depth from 0 to 1.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#endif
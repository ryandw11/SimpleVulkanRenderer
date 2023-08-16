#pragma once
#ifndef VULKAN_PIPELINE_HOLDER_INTF_H
#define VULKAN_PIPELINE_HOLDER_INTF_H

#include "VulkanIncludes.hpp"

/// <summary>
/// An object that holds a "pipeline" so data can easily be shared to utility classes.
/// 
/// Contains: Physical Device, Device, Queue, Command Pool.
/// 
/// I don't know how I feel about this. It may be removed.
/// </summary>
class VulkanPipelineHolderIntf
{
public:
	virtual operator VkPhysicalDevice () = 0;
	virtual operator VkDevice () = 0;
	virtual operator VkQueue () = 0;
	virtual operator VkCommandPool () = 0;
};

#endif
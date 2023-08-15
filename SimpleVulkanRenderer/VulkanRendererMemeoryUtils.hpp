#pragma once
#ifndef VULKAN_RENDER_MEMORY_UTILS_H
#define VULKAN_RENDER_MEMORY_UTILS_H

#include <memory>
#include <stdexcept>

template<typename T>
inline void ValidateNonNull(const T* var, const std::string msg)
{
#ifdef VKR_FORCE_NULL_CHECK
	if (var == nullptr)
	{
		throw std::runtime_error(msg);
	}
#endif
}

template<typename T>
inline void ValidateNonNull(const std::shared_ptr<T> var, const std::string msg)
{
#ifdef VKR_FORCE_NULL_CHECK
	if (!var)
	{
		throw std::runtime_error(msg);
	}
#endif
}

#endif
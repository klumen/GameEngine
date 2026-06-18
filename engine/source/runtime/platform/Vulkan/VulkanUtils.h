#pragma once

#include "runtime/core/Macro.h"

#include <vulkan/vulkan.h>

// Macro to check and display Vulkan return results
#if defined(__ANDROID__)
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
}
#else
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	ASSERT(res == VK_SUCCESS);																			\
}
#endif

namespace Lumen
{

}
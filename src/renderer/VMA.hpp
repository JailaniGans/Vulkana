// ===================================================================
// Vulkan Memory Allocator (VMA) Wrapper
// ===================================================================
// Provides unified memory allocation for Vulkan resources.
// ===================================================================

#pragma once

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace Vulkana
{
    class VMAAllocator
    {
    public:
        static VmaAllocator& getInstance();
        static void initialize(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
        static void cleanup();

    private:
        static VmaAllocator s_allocator;
    };
} // namespace Vulkana

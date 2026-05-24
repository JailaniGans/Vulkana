// ===================================================================
// Vulkan Memory Allocator (VMA) Implementation
// ===================================================================

#include "VMA.hpp"

namespace Vulkana
{
    VmaAllocator VMAAllocator::s_allocator = nullptr;

    VmaAllocator& VMAAllocator::getInstance()
    {
        return s_allocator;
    }

    void VMAAllocator::initialize(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorInfo.instance = instance;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;

        vmaCreateAllocator(&allocatorInfo, &s_allocator);
    }

    void VMAAllocator::cleanup()
    {
        if (s_allocator)
        {
            vmaDestroyAllocator(s_allocator);
            s_allocator = nullptr;
        }
    }
} // namespace Vulkana

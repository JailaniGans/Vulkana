#pragma once

// Context - inisialisasi Vulkan: instance, debug, GPU, device, queue, VMA

#include <volk.h>
#include <vk_mem_alloc.h>

struct GLFWwindow;

namespace Vulkana {

class Context {
public:
    Context() = default;
    ~Context();

    void init(GLFWwindow* window);
    void cleanup();

    VkInstance instance() const { return m_instance; }
    VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
    VkDevice device() const { return m_device; }
    VmaAllocator allocator() const { return m_allocator; }
    uint32_t graphicsQueueIndex() const { return m_graphicsIndex; }
    VkQueue graphicsQueue() const { return m_graphicsQueue; }

private:
    void pickPhysicalDevice();
    void createDevice();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    uint32_t m_graphicsIndex = UINT32_MAX;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
};

}

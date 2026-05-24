#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class Window;

/**
 * Context - Instance Vulkan, surface, GPU, perangkat logis, dan VMA.
 * Komponen renderer pertama yang harus diinisialisasi.
 * Membuat surface sendiri agar bisa validasi present queue family.
 */
class Context {
public:
    Context();
    ~Context();

    bool init(Window* window);
    void cleanup();

    VkInstance           getInstance()         const { return m_instance; }
    VkSurfaceKHR         getSurface()          const { return m_surface; }
    VkPhysicalDevice     getPhysicalDevice()   const { return m_physicalDevice; }
    VkDevice             getDevice()           const { return m_device; }
    VkQueue              getGraphicsQueue()    const { return m_graphicsQueue; }
    VkQueue              getPresentQueue()     const { return m_presentQueue; }
    uint32_t             getGraphicsFamily()   const { return m_graphicsFamily; }
    uint32_t             getPresentFamily()    const { return m_presentFamily; }
    VmaAllocator         getAllocator()        const { return m_allocator; }

private:
    VkInstance       m_instance;
    VkSurfaceKHR     m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice         m_device;
    VkQueue          m_graphicsQueue;
    VkQueue          m_presentQueue;
    uint32_t         m_graphicsFamily;
    uint32_t         m_presentFamily;
    VmaAllocator     m_allocator;
    Window*          m_window;

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool findQueueFamilies(VkPhysicalDevice device);
    bool pickPhysicalDevice();
};

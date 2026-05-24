#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class Context;

/**
 * Buffer - Abstraksi buffer Vulkan dengan alokasi VMA.
 * Mendukung device-local dan host-visible. Upload via staging.
 */
class Buffer {
public:
    Buffer();
    ~Buffer();

    bool create(Context* context,
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VmaMemoryUsage memoryUsage);
    void destroy();

    void upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    void uploadStaging(Context* context,
                       const void* data,
                       VkDeviceSize size,
                       VkCommandPool commandPool,
                       VkQueue queue);

    VkBuffer      getBuffer()    const { return m_buffer; }
    VmaAllocation getAllocation() const { return m_allocation; }
    VkDeviceSize  getSize()      const { return m_size; }

private:
    Context*     m_context;
    VkBuffer     m_buffer;
    VmaAllocation m_allocation;
    VkDeviceSize m_size;
};

#pragma once

// Buffer - vertex, index, uniform buffer via VMA
//   upload() buat staging + copy ke device local

#include <volk.h>
#include <vk_mem_alloc.h>
#include <cstddef>
#include <cstdint>

namespace Vulkana {

class Buffer {
public:
    Buffer() = default;
    ~Buffer();

    void init(VmaAllocator alloc, VkDeviceSize size,
              VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags,
              const void* data = nullptr);
    void cleanup();

    void upload(const void* data, VkDeviceSize size);
    void* map();
    void unmap();

    VkBuffer handle() const { return m_buffer; }
    VmaAllocation allocation() const { return m_allocation; }
    VkDeviceSize size() const { return m_size; }

private:
    VmaAllocator m_alloc = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;
};

}

#include "renderer/Buffer.hpp"
#include <cassert>
#include <cstring>

namespace Vulkana {

Buffer::~Buffer()
{
    cleanup();
}

// ------------------------------------------------------------------
// init - buat buffer + alokasi memory via VMA
// ------------------------------------------------------------------
void Buffer::init(VmaAllocator alloc, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags,
                  const void* data)
{
    m_alloc = alloc;
    m_size = size;

    VkBufferCreateInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size = size;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ai{};
    ai.usage = VMA_MEMORY_USAGE_AUTO;
    ai.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT;
    if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        ai.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;

    VkResult res = vmaCreateBuffer(m_alloc, &bi, &ai, &m_buffer, &m_allocation, nullptr);
    assert(res == VK_SUCCESS && "Gagal buat buffer via VMA");

    if (data)
    {
        void* mapped = nullptr;
        vmaMapMemory(m_alloc, m_allocation, &mapped);
        memcpy(mapped, data, (size_t)size);
        vmaUnmapMemory(m_alloc, m_allocation);
    }
}

void Buffer::cleanup()
{
    if (m_buffer)
    {
        vmaDestroyBuffer(m_alloc, m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
    }
}

// ------------------------------------------------------------------
// upload - buat staging buffer, copy data ke buffer ini
// ------------------------------------------------------------------
void Buffer::upload(const void* data, VkDeviceSize size)
{
    if (size > m_size) size = m_size;

    void* mapped = map();
    memcpy(mapped, data, (size_t)size);
    unmap();
}

void* Buffer::map()
{
    void* mapped = nullptr;
    vmaMapMemory(m_alloc, m_allocation, &mapped);
    return mapped;
}

void Buffer::unmap()
{
    vmaUnmapMemory(m_alloc, m_allocation);
}

}

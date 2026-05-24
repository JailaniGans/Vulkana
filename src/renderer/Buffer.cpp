#include "renderer/Buffer.hpp"
#include "renderer/Context.hpp"
#include "core/Log.hpp"

#include <cstring>

Buffer::Buffer()
    : m_context(nullptr)
    , m_buffer(VK_NULL_HANDLE)
    , m_allocation(VK_NULL_HANDLE)
    , m_size(0)
{
}

Buffer::~Buffer() {
    destroy();
}

bool Buffer::create(Context* context,
                    VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VmaMemoryUsage memoryUsage)
{
    m_context = context;
    m_size = size;

    VkBufferCreateInfo infoBuffer{};
    infoBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    infoBuffer.size = size;
    infoBuffer.usage = usage;
    infoBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo infoAlokasi{};
    infoAlokasi.usage = memoryUsage;

    if (memoryUsage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST ||
        memoryUsage == VMA_MEMORY_USAGE_AUTO) {
        infoAlokasi.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                            VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VkResult hasil = vmaCreateBuffer(context->getAllocator(),
                                      &infoBuffer, &infoAlokasi,
                                      &m_buffer, &m_allocation, nullptr);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat buffer via VMA");
        return false;
    }

    return true;
}

void Buffer::destroy() {
    if (m_buffer != VK_NULL_HANDLE && m_context && m_context->getAllocator() != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_context->getAllocator(), m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}

void Buffer::upload(const void* data, VkDeviceSize size, VkDeviceSize offset) {
    void* dataTerpetakan = nullptr;
    vmaMapMemory(m_context->getAllocator(), m_allocation, &dataTerpetakan);
    memcpy(static_cast<char*>(dataTerpetakan) + offset, data, static_cast<size_t>(size));
    vmaUnmapMemory(m_context->getAllocator(), m_allocation);
}

void Buffer::uploadStaging(Context* context,
                           const void* data,
                           VkDeviceSize size,
                           VkCommandPool commandPool,
                           VkQueue queue)
{
    Buffer bufferStage;
    bufferStage.create(context, size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    bufferStage.upload(data, size, 0);

    VkCommandBufferAllocateInfo infoAlokasi{};
    infoAlokasi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    infoAlokasi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    infoAlokasi.commandPool = commandPool;
    infoAlokasi.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(context->getDevice(), &infoAlokasi, &cmdBuffer);

    VkCommandBufferBeginInfo infoMulai{};
    infoMulai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    infoMulai.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuffer, &infoMulai);

    VkBufferCopy regionCopy{};
    regionCopy.srcOffset = 0;
    regionCopy.dstOffset = 0;
    regionCopy.size = size;
    vkCmdCopyBuffer(cmdBuffer, bufferStage.getBuffer(), m_buffer, 1, &regionCopy);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo infoSubmit{};
    infoSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.commandBufferCount = 1;
    infoSubmit.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(queue, 1, &infoSubmit, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(context->getDevice(), commandPool, 1, &cmdBuffer);
    bufferStage.destroy();
}

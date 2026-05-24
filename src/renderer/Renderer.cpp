#include "renderer/Renderer.hpp"
#include "renderer/Context.hpp"
#include "renderer/Swapchain.hpp"
#include "core/Log.hpp"
#include <cassert>
#include <algorithm>

namespace Vulkana {

Renderer::~Renderer()
{
    cleanup();
}

// ------------------------------------------------------------------
// init - buat command pool, command buffers, sync objects
// ------------------------------------------------------------------
void Renderer::init(Context& context, Swapchain& swapchain)
{
    cleanup();

    m_context = &context;
    m_swapchain = &swapchain;
    m_device = context.device();
    m_currentImage = 0;

    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
    LOG_INFO("Renderer siap");
}

void Renderer::cleanup()
{
    if (m_device == VK_NULL_HANDLE) return;

    vkDeviceWaitIdle(m_device);

    for (auto& fence : m_inFlightFence)
        if (fence) vkDestroyFence(m_device, fence, nullptr);
    for (auto& sem : m_imageAvailable)
        if (sem) vkDestroySemaphore(m_device, sem, nullptr);
    for (auto& sem : m_renderFinished)
        if (sem) vkDestroySemaphore(m_device, sem, nullptr);

    if (m_cmdPool)
        vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
}

// ------------------------------------------------------------------
// beginFrame - acquire image, tunggu fence, mulai command buffer
// ------------------------------------------------------------------
bool Renderer::beginFrame()
{
    VkFence fence = m_inFlightFence[m_frameIndex];
    vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &fence);

    uint32_t imageIndex = 0;
    VkResult res = vkAcquireNextImageKHR(
        m_device, m_swapchain->handle(), UINT64_MAX,
        m_imageAvailable[m_frameIndex], VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // Akan di-handle oleh Engine
        return false;
    }
    assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);

    m_currentImage = imageIndex;

    // Reset & mulai command buffer
    VkCommandBuffer cmd = m_cmdBufs[imageIndex];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &bi);

    // Begin render pass
    VkExtent2D extent = m_swapchain->extent();
    VkRenderPassBeginInfo rp{};
    rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp.renderPass = m_swapchain->renderPass();
    rp.framebuffer = m_swapchain->framebuffers()[imageIndex];
    rp.renderArea.extent = extent;

    VkClearValue clears[2]{};
    clears[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    clears[1].depthStencil = {1.0f, 0};
    rp.clearValueCount = 2;
    rp.pClearValues = clears;

    vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

// ------------------------------------------------------------------
// endFrame - akhiri render pass, submit command buffer, present
//   return false jika swapchain perlu direcreate
// ------------------------------------------------------------------
bool Renderer::endFrame()
{
    VkCommandBuffer cmd = m_cmdBufs[m_currentImage];
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    // Submit
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &m_imageAvailable[m_frameIndex];
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &m_renderFinished[m_frameIndex];

    VkResult res = vkQueueSubmit(m_context->graphicsQueue(), 1, &si,
                                  m_inFlightFence[m_frameIndex]);
    assert(res == VK_SUCCESS && "Gagal submit queue");

    // Present
    VkSwapchainKHR swapchainHandle = m_swapchain->handle();
    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &m_renderFinished[m_frameIndex];
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchainHandle;
    pi.pImageIndices = &m_currentImage;

    res = vkQueuePresentKHR(m_context->graphicsQueue(), &pi);

    m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    return (res != VK_ERROR_OUT_OF_DATE_KHR && res != VK_SUBOPTIMAL_KHR);
}

// ------------------------------------------------------------------
// drawIndexed - record draw call ke command buffer aktif
// ------------------------------------------------------------------
void Renderer::drawIndexed(uint32_t indexCount, uint32_t instanceCount)
{
    VkCommandBuffer cmd = m_cmdBufs[m_currentImage];
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, 0, 0, 0);
}

// ------------------------------------------------------------------
// createCommandPool
// ------------------------------------------------------------------
void Renderer::createCommandPool()
{
    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.queueFamilyIndex = m_context->graphicsQueueIndex();
    ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult res = vkCreateCommandPool(m_device, &ci, nullptr, &m_cmdPool);
    assert(res == VK_SUCCESS && "Gagal buat command pool");
}

// ------------------------------------------------------------------
// createCommandBuffers - satu per image swapchain
// ------------------------------------------------------------------
void Renderer::createCommandBuffers()
{
    uint32_t count = (uint32_t)m_swapchain->images().size();
    m_cmdBufs.resize(count);

    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = m_cmdPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = count;

    VkResult res = vkAllocateCommandBuffers(m_device, &ai, m_cmdBufs.data());
    assert(res == VK_SUCCESS && "Gagal alokasi command buffers");
}

// ------------------------------------------------------------------
// createSyncObjects - semaphore & fence
// ------------------------------------------------------------------
void Renderer::createSyncObjects()
{
    VkSemaphoreCreateInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fi{};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult r1 = vkCreateSemaphore(m_device, &si, nullptr, &m_imageAvailable[i]);
        VkResult r2 = vkCreateSemaphore(m_device, &si, nullptr, &m_renderFinished[i]);
        VkResult r3 = vkCreateFence(m_device, &fi, nullptr, &m_inFlightFence[i]);
        assert(r1 == VK_SUCCESS && r2 == VK_SUCCESS && r3 == VK_SUCCESS);
    }
}

}

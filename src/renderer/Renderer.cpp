#include "renderer/Renderer.hpp"
#include "renderer/Context.hpp"
#include "renderer/Swapchain.hpp"
#include "core/Window.hpp"
#include "core/Log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Renderer::Renderer()
    : m_context(nullptr)
    , m_swapchain(nullptr)
    , m_window(nullptr)
    , m_renderPass(VK_NULL_HANDLE)
    , m_commandPool(VK_NULL_HANDLE)
    , m_currentFrame(0)
{
}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::init(Window* window) {
    m_window = window;
    Log::info("Renderer menginisialisasi...");

    m_context = new Context();
    if (!m_context->init(m_window)) {
        Log::error("Renderer: Gagal inisialisasi Context");
        return false;
    }

    m_swapchain = new Swapchain();
    if (!m_swapchain->buatSurfaceSwapchain(m_context, m_window)) {
        Log::error("Renderer: Gagal membuat surface/swapchain");
        return false;
    }

    if (!buatRenderPass(m_swapchain->getImageFormat())) return false;

    if (!m_swapchain->buatFramebuffers(m_renderPass)) {
        Log::error("Renderer: Gagal membuat framebuffer");
        return false;
    }

    if (!buatSyncObjects(m_swapchain->getImageCount())) return false;
    if (!buatCommandPool()) return false;
    if (!buatCommandBuffers()) return false;

    Log::info("Renderer berhasil diinisialisasi");
    return true;
}

bool Renderer::beginFrame(uint32_t& imageIndex, VkCommandBuffer& cmdBuffer) {
    // Tunggu GPU frame sebelumnya selesai
    vkWaitForFences(m_context->getDevice(), 1,
                    &m_fenceDalamProses[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_context->getDevice(), 1,
                  &m_fenceDalamProses[m_currentFrame]);

    // Akuisisi gambar — pakai fence agar tidak perlu semaphore per image
    // (menghindari VUID-vkQueueSubmit-pSignalSemaphores-00067)
    VkResult hasil = vkAcquireNextImageKHR(m_context->getDevice(),
                                            m_swapchain->getSwapchain(),
                                            UINT64_MAX,
                                            VK_NULL_HANDLE,
                                            m_fenceDalamProses[m_currentFrame],
                                            &imageIndex);

    if (hasil == VK_ERROR_OUT_OF_DATE_KHR || hasil == VK_SUBOPTIMAL_KHR) {
        int lebar = 0, tinggi = 0;
        glfwGetFramebufferSize(m_window->getHandle(), &lebar, &tinggi);
        if (lebar == 0 || tinggi == 0) return false;

        vkDeviceWaitIdle(m_context->getDevice());
        if (m_swapchain->recreate(m_renderPass)) {
            vkFreeCommandBuffers(m_context->getDevice(), m_commandPool,
                                 static_cast<uint32_t>(m_commandBuffers.size()),
                                 m_commandBuffers.data());
            buatCommandBuffers();
            // Re-alokasi semaphore untuk jumlah image baru
            uint32_t jumlahBaru = m_swapchain->getImageCount();
            for (auto sem : m_semaphoreRenderSelesai)
                vkDestroySemaphore(m_context->getDevice(), sem, nullptr);
            m_semaphoreRenderSelesai.resize(jumlahBaru);
            VkSemaphoreCreateInfo infoSem{};
            infoSem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            for (uint32_t i = 0; i < jumlahBaru; i++)
                vkCreateSemaphore(m_context->getDevice(), &infoSem, nullptr, &m_semaphoreRenderSelesai[i]);
        }
        return false;
    } else if (hasil != VK_SUCCESS) {
        Log::error("Gagal mengakuisisi gambar swapchain");
        return false;
    }

    // Tunggu gambar benar-benar siap (fence dari acquire di atas)
    vkWaitForFences(m_context->getDevice(), 1,
                    &m_fenceDalamProses[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_context->getDevice(), 1,
                  &m_fenceDalamProses[m_currentFrame]);

    // Mulai command buffer
    cmdBuffer = m_commandBuffers[imageIndex];

    VkCommandBufferBeginInfo infoMulai{};
    infoMulai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    infoMulai.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkResetCommandBuffer(cmdBuffer, 0);
    vkBeginCommandBuffer(cmdBuffer, &infoMulai);

    VkClearValue nilaiBersih[2]{};
    nilaiBersih[0].color = {{0.1f, 0.1f, 0.2f, 1.0f}};
    nilaiBersih[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo infoRP{};
    infoRP.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    infoRP.renderPass = m_renderPass;
    infoRP.framebuffer = m_swapchain->getFramebuffers()[imageIndex];
    infoRP.renderArea.offset = {0, 0};
    infoRP.renderArea.extent = m_swapchain->getExtent();
    infoRP.clearValueCount = 2;
    infoRP.pClearValues = nilaiBersih;

    vkCmdBeginRenderPass(cmdBuffer, &infoRP, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = static_cast<float>(m_swapchain->getExtent().width);
    viewport.height = static_cast<float>(m_swapchain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain->getExtent();
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    return true;
}

void Renderer::endFrame(uint32_t imageIndex, VkCommandBuffer cmdBuffer) {
    vkCmdEndRenderPass(cmdBuffer);
    vkEndCommandBuffer(cmdBuffer);

    // Submit — tidak perlu wait semaphore karena fence sudah menjamin
    // image siap. Signal render selesai via semaphore per image.
    VkPipelineStageFlags stageTunggu[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo infoSubmit{};
    infoSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.waitSemaphoreCount = 0;
    infoSubmit.pWaitSemaphores = nullptr;
    infoSubmit.pWaitDstStageMask = stageTunggu;
    infoSubmit.commandBufferCount = 1;
    infoSubmit.pCommandBuffers = &cmdBuffer;
    infoSubmit.signalSemaphoreCount = 1;
    infoSubmit.pSignalSemaphores = &m_semaphoreRenderSelesai[imageIndex];

    VkResult hasil = vkQueueSubmit(m_context->getGraphicsQueue(), 1, &infoSubmit,
                                   m_fenceDalamProses[m_currentFrame]);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal submit perintah");
    }

    // Present — tunggu render selesai via semaphore image-specific
    VkPresentInfoKHR infoPresent{};
    infoPresent.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    infoPresent.waitSemaphoreCount = 1;
    infoPresent.pWaitSemaphores = &m_semaphoreRenderSelesai[imageIndex];
    VkSwapchainKHR swapchain = m_swapchain->getSwapchain();
    infoPresent.swapchainCount = 1;
    infoPresent.pSwapchains = &swapchain;
    infoPresent.pImageIndices = &imageIndex;

    hasil = vkQueuePresentKHR(m_context->getPresentQueue(), &infoPresent);
    if (hasil == VK_ERROR_OUT_OF_DATE_KHR || hasil == VK_SUBOPTIMAL_KHR) {
        // beginFrame akan handle recreate
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::waitIdle() {
    if (m_context && m_context->getDevice() != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_context->getDevice());
    }
}

void Renderer::cleanup() {
    waitIdle();

    for (auto sem : m_semaphoreRenderSelesai)
        vkDestroySemaphore(m_context->getDevice(), sem, nullptr);
    for (auto fence : m_fenceDalamProses)
        vkDestroyFence(m_context->getDevice(), fence, nullptr);

    if (m_commandPool != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(m_context->getDevice(), m_commandPool,
                             static_cast<uint32_t>(m_commandBuffers.size()),
                             m_commandBuffers.data());
        m_commandBuffers.clear();
        vkDestroyCommandPool(m_context->getDevice(), m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }

    delete m_swapchain; m_swapchain = nullptr;
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_context->getDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
    delete m_context; m_context = nullptr;

    Log::info("Renderer dibersihkan");
}

bool Renderer::buatRenderPass(VkFormat formatGambar) {
    VkAttachmentDescription lampiranWarna{};
    lampiranWarna.format = formatGambar;
    lampiranWarna.samples = VK_SAMPLE_COUNT_1_BIT;
    lampiranWarna.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    lampiranWarna.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    lampiranWarna.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    lampiranWarna.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    lampiranWarna.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    lampiranWarna.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription lampiranDepth{};
    lampiranDepth.format = VK_FORMAT_D32_SFLOAT;
    lampiranDepth.samples = VK_SAMPLE_COUNT_1_BIT;
    lampiranDepth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    lampiranDepth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    lampiranDepth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    lampiranDepth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    lampiranDepth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    lampiranDepth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference refWarna{};
    refWarna.attachment = 0; refWarna.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference refDepth{};
    refDepth.attachment = 1; refDepth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &refWarna;
    subpass.pDepthStencilAttachment = &refDepth;

    VkSubpassDependency dependensi{};
    dependensi.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependensi.dstSubpass = 0;
    dependensi.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependensi.srcAccessMask = 0;
    dependensi.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependensi.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription lampiran[] = { lampiranWarna, lampiranDepth };

    VkRenderPassCreateInfo infoRP{};
    infoRP.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    infoRP.attachmentCount = 2;
    infoRP.pAttachments = lampiran;
    infoRP.subpassCount = 1;
    infoRP.pSubpasses = &subpass;
    infoRP.dependencyCount = 1;
    infoRP.pDependencies = &dependensi;

    VkResult hasil = vkCreateRenderPass(m_context->getDevice(), &infoRP, nullptr, &m_renderPass);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat render pass");
        return false;
    }
    Log::info("Render pass dibuat");
    return true;
}

bool Renderer::buatCommandPool() {
    VkCommandPoolCreateInfo infoPool{};
    infoPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    infoPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    infoPool.queueFamilyIndex = m_context->getGraphicsFamily();

    VkResult hasil = vkCreateCommandPool(m_context->getDevice(), &infoPool, nullptr, &m_commandPool);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat command pool");
        return false;
    }
    Log::info("Command pool dibuat");
    return true;
}

bool Renderer::buatCommandBuffers() {
    m_commandBuffers.resize(m_swapchain->getFramebuffers().size());

    VkCommandBufferAllocateInfo infoAlokasi{};
    infoAlokasi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    infoAlokasi.commandPool = m_commandPool;
    infoAlokasi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    infoAlokasi.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    VkResult hasil = vkAllocateCommandBuffers(m_context->getDevice(), &infoAlokasi, m_commandBuffers.data());
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal mengalokasikan command buffer");
        return false;
    }
    Log::info("Command buffer dibuat");
    return true;
}

bool Renderer::buatSyncObjects(uint32_t jumlahGambar) {
    // Semaphore render-selesai per swapchain image — index dengan imageIndex
    // Aman dari konflik reuse karena setiap image hanya dipresent sekali per siklus
    m_semaphoreRenderSelesai.resize(jumlahGambar);

    // Fence per frame in-flight (MAX_FRAMES_IN_FLIGHT)
    m_fenceDalamProses.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo infoSem{};
    infoSem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t i = 0; i < jumlahGambar; i++) {
        VkResult hasil = vkCreateSemaphore(m_context->getDevice(), &infoSem, nullptr,
                                            &m_semaphoreRenderSelesai[i]);
        if (hasil != VK_SUCCESS) {
            Log::error("Gagal membuat semaphore render selesai");
            return false;
        }
    }

    VkFenceCreateInfo infoFence{};
    infoFence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    infoFence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult hasil = vkCreateFence(m_context->getDevice(), &infoFence, nullptr,
                                        &m_fenceDalamProses[i]);
        if (hasil != VK_SUCCESS) {
            Log::error("Gagal membuat fence");
            return false;
        }
    }

    Log::info("Objek sinkronisasi dibuat");
    return true;
}

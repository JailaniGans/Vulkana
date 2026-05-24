#include "renderer/Swapchain.hpp"
#include "renderer/Context.hpp"
#include "core/Window.hpp"
#include "core/Log.hpp"

#include <algorithm>
#include <limits>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

Swapchain::Swapchain()
    : m_context(nullptr)
    , m_window(nullptr)
    , m_surface(VK_NULL_HANDLE)
    , m_swapchain(VK_NULL_HANDLE)
    , m_imageFormat(VK_FORMAT_UNDEFINED)
    , m_extent{0, 0}
    , m_depthImage(VK_NULL_HANDLE)
    , m_depthAllocation(VK_NULL_HANDLE)
    , m_depthImageView(VK_NULL_HANDLE)
    , m_renderPass(VK_NULL_HANDLE)
{
}

Swapchain::~Swapchain() {
    destroy();
}

bool Swapchain::buatSurfaceSwapchain(Context* context, Window* window) {
    m_context = context;
    m_window = window;

    // Surface sudah dibuat oleh Context — kita pakai langsung
    m_surface = context->getSurface();
    if (m_surface == VK_NULL_HANDLE) {
        Log::error("Surface dari Context tidak valid");
        return false;
    }

    VkResult hasil;

    // Cari format surface + mode present + extent
    VkSurfaceCapabilitiesKHR kapabilitas;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->getPhysicalDevice(), m_surface, &kapabilitas);

    uint32_t jumlahGambar = kapabilitas.minImageCount + 1;
    if (kapabilitas.maxImageCount > 0 && jumlahGambar > kapabilitas.maxImageCount) {
        jumlahGambar = kapabilitas.maxImageCount;
    }

    VkSurfaceFormatKHR formatPermukaan = pilihFormatSurface();
    m_imageFormat = formatPermukaan.format;
    VkPresentModeKHR modePresent = pilihModePresent();
    m_extent = pilihExtent(kapabilitas);

    // Buat swapchain
    VkSwapchainCreateInfoKHR infoSwapchain{};
    infoSwapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    infoSwapchain.surface = m_surface;
    infoSwapchain.minImageCount = jumlahGambar;
    infoSwapchain.imageFormat = formatPermukaan.format;
    infoSwapchain.imageColorSpace = formatPermukaan.colorSpace;
    infoSwapchain.imageExtent = m_extent;
    infoSwapchain.imageArrayLayers = 1;
    infoSwapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    infoSwapchain.preTransform = kapabilitas.currentTransform;
    infoSwapchain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    infoSwapchain.presentMode = modePresent;
    infoSwapchain.clipped = VK_TRUE;
    infoSwapchain.oldSwapchain = VK_NULL_HANDLE;

    hasil = vkCreateSwapchainKHR(context->getDevice(), &infoSwapchain, nullptr, &m_swapchain);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat swapchain");
        return false;
    }
    Log::info("Swapchain dibuat");

    // Ambil gambar-gambar swapchain
    uint32_t jumlahGambarAktual = 0;
    vkGetSwapchainImagesKHR(context->getDevice(), m_swapchain, &jumlahGambarAktual, nullptr);
    m_images.resize(jumlahGambarAktual);
    vkGetSwapchainImagesKHR(context->getDevice(), m_swapchain, &jumlahGambarAktual, m_images.data());

    // Buat image view untuk setiap gambar
    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo infoView{};
        infoView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        infoView.image = m_images[i];
        infoView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        infoView.format = m_imageFormat;
        infoView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        infoView.subresourceRange.baseMipLevel = 0;
        infoView.subresourceRange.levelCount = 1;
        infoView.subresourceRange.baseArrayLayer = 0;
        infoView.subresourceRange.layerCount = 1;

        hasil = vkCreateImageView(context->getDevice(), &infoView, nullptr, &m_imageViews[i]);
        if (hasil != VK_SUCCESS) {
            Log::error("Gagal membuat image view");
            return false;
        }
    }
    Log::info("Image view dibuat");

    // Depth resources (belum butuh render pass)
    if (!buatDepthResources()) {
        Log::error("Gagal membuat resource depth");
        return false;
    }

    // Framebuffer dibuat nanti setelah render pass tersedia
    Log::info("Surface + swapchain siap (framebuffer menyusul)");
    return true;
}

bool Swapchain::buatFramebuffers(VkRenderPass renderPass) {
    m_renderPass = renderPass;
    VkDevice perangkat = m_context->getDevice();

    // Hapus framebuffer lama jika ada
    for (auto fb : m_framebuffers) {
        vkDestroyFramebuffer(perangkat, fb, nullptr);
    }
    m_framebuffers.clear();

    m_framebuffers.resize(m_imageViews.size());
    for (size_t i = 0; i < m_imageViews.size(); i++) {
        VkImageView lampiran[] = { m_imageViews[i], m_depthImageView };

        VkFramebufferCreateInfo infoFB{};
        infoFB.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        infoFB.renderPass = m_renderPass;
        infoFB.attachmentCount = 2;
        infoFB.pAttachments = lampiran;
        infoFB.width = m_extent.width;
        infoFB.height = m_extent.height;
        infoFB.layers = 1;

        VkResult hasil = vkCreateFramebuffer(perangkat, &infoFB, nullptr, &m_framebuffers[i]);
        if (hasil != VK_SUCCESS) {
            Log::error("Gagal membuat framebuffer");
            return false;
        }
    }
    Log::info("Framebuffer dibuat");
    return true;
}

void Swapchain::destroy() {
    VkDevice perangkat = m_context ? m_context->getDevice() : VK_NULL_HANDLE;
    if (perangkat == VK_NULL_HANDLE) return;

    for (auto fb : m_framebuffers) {
        vkDestroyFramebuffer(perangkat, fb, nullptr);
    }
    m_framebuffers.clear();

    if (m_depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(perangkat, m_depthImageView, nullptr);
        m_depthImageView = VK_NULL_HANDLE;
    }
    if (m_depthImage != VK_NULL_HANDLE && m_context) {
        vmaDestroyImage(m_context->getAllocator(), m_depthImage, m_depthAllocation);
        m_depthImage = VK_NULL_HANDLE;
        m_depthAllocation = VK_NULL_HANDLE;
    }

    for (auto iv : m_imageViews) {
        vkDestroyImageView(perangkat, iv, nullptr);
    }
    m_imageViews.clear();
    m_images.clear();

    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(perangkat, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    // Surface tidak dihancurkan di sini — dimiliki oleh Context
    m_surface = VK_NULL_HANDLE;
}

bool Swapchain::recreate(VkRenderPass renderPass) {
    VkDevice perangkat = m_context->getDevice();

    // Cek extent — skip jika 0 (jendela di-minimize)
    VkSurfaceCapabilitiesKHR kapabilitas;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->getPhysicalDevice(), m_surface, &kapabilitas);
    VkExtent2D ukuranBaru = pilihExtent(kapabilitas);
    if (ukuranBaru.width == 0 || ukuranBaru.height == 0) {
        m_extent = ukuranBaru;
        Log::info("Rekreasi swapchain ditunda: extent 0");
        return false;
    }

    // Simpan resource lama untuk dihancurkan setelah sukses
    VkSwapchainKHR swapchainLama = m_swapchain;
    std::vector<VkFramebuffer> framebufferLama = std::move(m_framebuffers);
    std::vector<VkImageView> imageViewLama = std::move(m_imageViews);
    VkImageView depthImageViewLama = m_depthImageView;
    VkImage depthImageLama = m_depthImage;
    VmaAllocation depthAlokasiLama = m_depthAllocation;

    // Set default agar cleanup tidak ganggu resource baru
    m_framebuffers.clear();
    m_imageViews.clear();
    m_depthImageView = VK_NULL_HANDLE;
    m_depthImage = VK_NULL_HANDLE;
    m_depthAllocation = VK_NULL_HANDLE;

    // Buat swapchain baru dulu
    VkSurfaceFormatKHR formatPermukaan = pilihFormatSurface();
    VkPresentModeKHR modePresent = pilihModePresent();

    uint32_t jumlahGambar = kapabilitas.minImageCount + 1;
    if (kapabilitas.maxImageCount > 0 && jumlahGambar > kapabilitas.maxImageCount) {
        jumlahGambar = kapabilitas.maxImageCount;
    }

    VkSwapchainCreateInfoKHR infoSwapchain{};
    infoSwapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    infoSwapchain.surface = m_surface;
    infoSwapchain.minImageCount = jumlahGambar;
    infoSwapchain.imageFormat = formatPermukaan.format;
    infoSwapchain.imageColorSpace = formatPermukaan.colorSpace;
    infoSwapchain.imageExtent = ukuranBaru;
    infoSwapchain.imageArrayLayers = 1;
    infoSwapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    infoSwapchain.preTransform = kapabilitas.currentTransform;
    infoSwapchain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    infoSwapchain.presentMode = modePresent;
    infoSwapchain.clipped = VK_TRUE;
    infoSwapchain.oldSwapchain = swapchainLama;

    VkSwapchainKHR swapchainBaru = VK_NULL_HANDLE;
    VkResult hasil = vkCreateSwapchainKHR(perangkat, &infoSwapchain, nullptr, &swapchainBaru);
    if (hasil != VK_SUCCESS) {
        Log::error("Rekreasi swapchain gagal");
        // Kembalikan resource lama
        m_swapchain = swapchainLama;
        m_framebuffers = std::move(framebufferLama);
        m_imageViews = std::move(imageViewLama);
        m_depthImageView = depthImageViewLama;
        m_depthImage = depthImageLama;
        m_depthAllocation = depthAlokasiLama;
        return false;
    }

    // Swapchain baru berhasil — hancurkan resource lama
    for (auto fb : framebufferLama) vkDestroyFramebuffer(perangkat, fb, nullptr);
    for (auto iv : imageViewLama) vkDestroyImageView(perangkat, iv, nullptr);
    if (depthImageViewLama != VK_NULL_HANDLE)
        vkDestroyImageView(perangkat, depthImageViewLama, nullptr);
    if (depthImageLama != VK_NULL_HANDLE && m_context)
        vmaDestroyImage(m_context->getAllocator(), depthImageLama, depthAlokasiLama);

    m_swapchain = swapchainBaru;
    vkDestroySwapchainKHR(perangkat, swapchainLama, nullptr);

    // Update format + extent
    m_imageFormat = formatPermukaan.format;
    m_extent = ukuranBaru;

    // Ambil gambar baru
    uint32_t jumlahGambarAktual = 0;
    vkGetSwapchainImagesKHR(perangkat, m_swapchain, &jumlahGambarAktual, nullptr);
    m_images.resize(jumlahGambarAktual);
    vkGetSwapchainImagesKHR(perangkat, m_swapchain, &jumlahGambarAktual, m_images.data());

    // Image view baru
    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo infoView{};
        infoView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        infoView.image = m_images[i];
        infoView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        infoView.format = m_imageFormat;
        infoView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        infoView.subresourceRange.baseMipLevel = 0;
        infoView.subresourceRange.levelCount = 1;
        infoView.subresourceRange.baseArrayLayer = 0;
        infoView.subresourceRange.layerCount = 1;

        hasil = vkCreateImageView(perangkat, &infoView, nullptr, &m_imageViews[i]);
        if (hasil != VK_SUCCESS) return false;
    }

    // Depth baru
    if (!buatDepthResources()) {
        Log::error("Gagal membuat ulang resource depth");
        return false;
    }

    // Framebuffer baru
    m_framebuffers.resize(m_imageViews.size());
    for (size_t i = 0; i < m_imageViews.size(); i++) {
        VkImageView lampiran[] = { m_imageViews[i], m_depthImageView };

        VkFramebufferCreateInfo infoFB{};
        infoFB.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        infoFB.renderPass = renderPass;
        infoFB.attachmentCount = 2;
        infoFB.pAttachments = lampiran;
        infoFB.width = m_extent.width;
        infoFB.height = m_extent.height;
        infoFB.layers = 1;

        hasil = vkCreateFramebuffer(perangkat, &infoFB, nullptr, &m_framebuffers[i]);
        if (hasil != VK_SUCCESS) return false;
    }

    return true;
}

VkSurfaceFormatKHR Swapchain::pilihFormatSurface() {
    uint32_t jumlahFormat = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->getPhysicalDevice(), m_surface, &jumlahFormat, nullptr);
    std::vector<VkSurfaceFormatKHR> formatTersedia(jumlahFormat);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->getPhysicalDevice(), m_surface, &jumlahFormat, formatTersedia.data());

    for (const auto& fmt : formatTersedia) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
            fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return fmt;
        }
    }

    return formatTersedia[0];
}

VkPresentModeKHR Swapchain::pilihModePresent() {
    uint32_t jumlahMode = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->getPhysicalDevice(), m_surface, &jumlahMode, nullptr);
    std::vector<VkPresentModeKHR> modeTersedia(jumlahMode);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->getPhysicalDevice(), m_surface, &jumlahMode, modeTersedia.data());

    for (const auto& mode : modeTersedia) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::pilihExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    int lebar, tinggi;
    glfwGetFramebufferSize(m_window->getHandle(), &lebar, &tinggi);

    VkExtent2D ukuran = {
        static_cast<uint32_t>(lebar),
        static_cast<uint32_t>(tinggi)
    };

    ukuran.width = std::clamp(ukuran.width,
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    ukuran.height = std::clamp(ukuran.height,
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return ukuran;
}

bool Swapchain::buatDepthResources() {
    VkFormat formatDepth = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo infoGambar{};
    infoGambar.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    infoGambar.imageType = VK_IMAGE_TYPE_2D;
    infoGambar.extent.width = m_extent.width;
    infoGambar.extent.height = m_extent.height;
    infoGambar.extent.depth = 1;
    infoGambar.mipLevels = 1;
    infoGambar.arrayLayers = 1;
    infoGambar.format = formatDepth;
    infoGambar.tiling = VK_IMAGE_TILING_OPTIMAL;
    infoGambar.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    infoGambar.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    infoGambar.samples = VK_SAMPLE_COUNT_1_BIT;
    infoGambar.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo infoAlokasi{};
    infoAlokasi.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkResult hasil = vmaCreateImage(m_context->getAllocator(),
                                     &infoGambar, &infoAlokasi,
                                     &m_depthImage, &m_depthAllocation, nullptr);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat depth image via VMA");
        return false;
    }

    VkImageViewCreateInfo infoView{};
    infoView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    infoView.image = m_depthImage;
    infoView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    infoView.format = formatDepth;
    infoView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    infoView.subresourceRange.baseMipLevel = 0;
    infoView.subresourceRange.levelCount = 1;
    infoView.subresourceRange.baseArrayLayer = 0;
    infoView.subresourceRange.layerCount = 1;

    hasil = vkCreateImageView(m_context->getDevice(), &infoView, nullptr, &m_depthImageView);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat depth image view");
        return false;
    }

    return true;
}

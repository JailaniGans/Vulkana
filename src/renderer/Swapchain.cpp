#include "renderer/Swapchain.hpp"
#include "core/Log.hpp"
#include <cassert>
#include <algorithm>
#include <limits>

namespace Vulkana {

Swapchain::~Swapchain()
{
    cleanup();
}

// ------------------------------------------------------------------
// init - buat swapchain dan semua resources terkait
// ------------------------------------------------------------------
void Swapchain::init(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface,
                     int width, int height)
{
    m_device = device;
    m_gpu = gpu;
    m_surface = surface;

    createSwapchain(width, height);
    createImageViews();
    createDepthResources();
    createRenderPass();
    createFramebuffers();
    LOG_INFO("Swapchain siap");
}

void Swapchain::cleanup()
{
    for (auto fb : m_framebuffers)
        vkDestroyFramebuffer(m_device, fb, nullptr);
    m_framebuffers.clear();

    if (m_renderPass)
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    if (m_depthView)
        vkDestroyImageView(m_device, m_depthView, nullptr);
    if (m_depthImage)
        vkDestroyImage(m_device, m_depthImage, nullptr);
    if (m_depthMemory)
        vkFreeMemory(m_device, m_depthMemory, nullptr);

    for (auto& img : m_images)
    {
        if (img.view) vkDestroyImageView(m_device, img.view, nullptr);
    }
    m_images.clear();

    if (m_swapchain)
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void Swapchain::recreate(int width, int height)
{
    vkDeviceWaitIdle(m_device);
    cleanup();
    createSwapchain(width, height);
    createImageViews();
    createDepthResources();
    createRenderPass();
    createFramebuffers();
    LOG_INFO("Swapchain direcreate");
}

// ------------------------------------------------------------------
// Pilih format swapchain (SDR 8-bit preferred)
// ------------------------------------------------------------------
static VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (auto& f : formats)
    {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    }
    return formats[0];
}

// ------------------------------------------------------------------
// Pilih mode present (mailbox > fifo)
// ------------------------------------------------------------------
static VkPresentModeKHR chooseMode(const std::vector<VkPresentModeKHR>& modes)
{
    for (auto& m : modes)
    {
        if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

// ------------------------------------------------------------------
// Pilih extent (ukuran framebuffer)
// ------------------------------------------------------------------
static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& caps, int w, int h)
{
    if (caps.currentExtent.width != UINT32_MAX)
        return caps.currentExtent;

    VkExtent2D e{};
    e.width  = std::clamp((uint32_t)w, caps.minImageExtent.width,  caps.maxImageExtent.width);
    e.height = std::clamp((uint32_t)h, caps.minImageExtent.height, caps.maxImageExtent.height);
    return e;
}

void Swapchain::createSwapchain(int w, int h)
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &caps);

    uint32_t fmtCount = 0, modeCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &fmtCount, nullptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &modeCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount);
    std::vector<VkPresentModeKHR> modes(modeCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &fmtCount, formats.data());
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &modeCount, modes.data());

    VkSurfaceFormatKHR fmt = chooseFormat(formats);
    VkPresentModeKHR mode = chooseMode(modes);
    m_extent = chooseExtent(caps, w, h);
    m_format = fmt.format;

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

    VkSwapchainCreateInfoKHR si{};
    si.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    si.surface = m_surface;
    si.minImageCount = imageCount;
    si.imageFormat = fmt.format;
    si.imageColorSpace = fmt.colorSpace;
    si.imageExtent = m_extent;
    si.imageArrayLayers = 1;
    si.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    si.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    si.preTransform = caps.currentTransform;
    si.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    si.presentMode = mode;
    si.clipped = VK_TRUE;

    VkResult res = vkCreateSwapchainKHR(m_device, &si, nullptr, &m_swapchain);
    assert(res == VK_SUCCESS && "Gagal buat swapchain");
}

void Swapchain::createImageViews()
{
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, images.data());

    m_images.resize(count);
    for (size_t i = 0; i < count; i++)
    {
        m_images[i].image = images[i];

        VkImageViewCreateInfo vi{};
        vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = images[i];
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = m_format;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.layerCount = 1;

        VkResult res = vkCreateImageView(m_device, &vi, nullptr, &m_images[i].view);
        assert(res == VK_SUCCESS && "Gagal buat image view");
    }
}

void Swapchain::createDepthResources()
{
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo ii{};
    ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = depthFormat;
    ii.extent = {m_extent.width, m_extent.height, 1};
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult res = vkCreateImage(m_device, &ii, nullptr, &m_depthImage);
    assert(res == VK_SUCCESS && "Gagal buat depth image");

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(m_device, m_depthImage, &req);

    VkMemoryAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = 0; // TODO: cari memory type sesuai properti

    // Cari memory type yang support VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        if ((req.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            ai.memoryTypeIndex = i;
            break;
        }
    }

    res = vkAllocateMemory(m_device, &ai, nullptr, &m_depthMemory);
    assert(res == VK_SUCCESS && "Gagal alokasi depth memory");

    vkBindImageMemory(m_device, m_depthImage, m_depthMemory, 0);

    VkImageViewCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vi.image = m_depthImage;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = depthFormat;
    vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vi.subresourceRange.levelCount = 1;
    vi.subresourceRange.layerCount = 1;

    res = vkCreateImageView(m_device, &vi, nullptr, &m_depthView);
    assert(res == VK_SUCCESS && "Gagal buat depth image view");
}

void Swapchain::createRenderPass()
{
    VkAttachmentDescription colorAtt{};
    colorAtt.format = m_format;
    colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAtt{};
    depthAtt.format = VK_FORMAT_D32_SFLOAT;
    depthAtt.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAtt.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAtt, depthAtt};

    VkRenderPassCreateInfo rp{};
    rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp.attachmentCount = 2;
    rp.pAttachments = attachments;
    rp.subpassCount = 1;
    rp.pSubpasses = &subpass;
    rp.dependencyCount = 1;
    rp.pDependencies = &dep;

    VkResult res = vkCreateRenderPass(m_device, &rp, nullptr, &m_renderPass);
    assert(res == VK_SUCCESS && "Gagal buat render pass");
}

void Swapchain::createFramebuffers()
{
    m_framebuffers.resize(m_images.size());

    for (size_t i = 0; i < m_images.size(); i++)
    {
        VkImageView attachments[] = {m_images[i].view, m_depthView};

        VkFramebufferCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fi.renderPass = m_renderPass;
        fi.attachmentCount = 2;
        fi.pAttachments = attachments;
        fi.width = m_extent.width;
        fi.height = m_extent.height;
        fi.layers = 1;

        VkResult res = vkCreateFramebuffer(m_device, &fi, nullptr, &m_framebuffers[i]);
        assert(res == VK_SUCCESS && "Gagal buat framebuffer");
    }
}

}

#include "renderer/Context.hpp"
#include "core/Log.hpp"
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>

namespace Vulkana {

// ------------------------------------------------------------------
// Callback debug messenger validation layer
// ------------------------------------------------------------------
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOG_ERROR(data->pMessage);
    return VK_FALSE;
}

Context::~Context()
{
    cleanup();
}

// ------------------------------------------------------------------
// init - inisialisasi Volk, instance, debug, GPU, device, VMA
// ------------------------------------------------------------------
void Context::init(GLFWwindow* window)
{
    // Load Vulkan loader via Volk
    VkResult result = volkInitialize();
    assert(result == VK_SUCCESS && "Gagal load Vulkan loader");

    // Vulkan instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkana";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "Vulkana";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t glfwCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwCount);
    std::vector<const char*> exts(glfwExts, glfwExts + glfwCount);
    exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::vector<const char*> layers;
#ifdef _DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    VkInstanceCreateInfo instInfo{};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = (uint32_t)exts.size();
    instInfo.ppEnabledExtensionNames = exts.data();
    instInfo.enabledLayerCount = (uint32_t)layers.size();
    instInfo.ppEnabledLayerNames = layers.data();

    result = vkCreateInstance(&instInfo, nullptr, &m_instance);
    assert(result == VK_SUCCESS && "Gagal buat instance Vulkan");

    volkLoadInstance(m_instance);
    LOG_INFO("Instance Vulkan dibuat");

    // Debug messenger
    VkDebugUtilsMessengerCreateInfoEXT dbgInfo{};
    dbgInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbgInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbgInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbgInfo.pfnUserCallback = debugCallback;

    auto createDbg = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (createDbg)
    {
        createDbg(m_instance, &dbgInfo, nullptr, &m_debugMessenger);
        LOG_INFO("Debug messenger aktif");
    }

    // Pilih GPU & buat device
    pickPhysicalDevice();
    createDevice();

    volkLoadDevice(m_device);
    LOG_INFO("Device Vulkan siap");

    // VMA allocator dengan function pointer dari Volk
    VmaVulkanFunctions vf{};
    vf.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vf.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vf.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vf.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vf.vkAllocateMemory = vkAllocateMemory;
    vf.vkFreeMemory = vkFreeMemory;
    vf.vkMapMemory = vkMapMemory;
    vf.vkUnmapMemory = vkUnmapMemory;
    vf.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vf.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vf.vkBindBufferMemory = vkBindBufferMemory;
    vf.vkBindImageMemory = vkBindImageMemory;
    vf.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vf.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vf.vkCreateBuffer = vkCreateBuffer;
    vf.vkDestroyBuffer = vkDestroyBuffer;
    vf.vkCreateImage = vkCreateImage;
    vf.vkDestroyImage = vkDestroyImage;
    vf.vkCmdCopyBuffer = vkCmdCopyBuffer;
    vf.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
    vf.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
    vf.vkBindBufferMemory2KHR = vkBindBufferMemory2;
    vf.vkBindImageMemory2KHR = vkBindImageMemory2;
    vf.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    vf.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    vf.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

    VmaAllocatorCreateInfo ai{};
    ai.vulkanApiVersion = VK_API_VERSION_1_3;
    ai.physicalDevice = m_physicalDevice;
    ai.device = m_device;
    ai.instance = m_instance;
    ai.pVulkanFunctions = &vf;

    result = vmaCreateAllocator(&ai, &m_allocator);
    assert(result == VK_SUCCESS && "Gagal buat VMA allocator");
    LOG_INFO("VMA allocator siap");
}

void Context::cleanup()
{
    if (m_allocator)        vmaDestroyAllocator(m_allocator);
    if (m_device)            vkDestroyDevice(m_device, nullptr);
    if (m_debugMessenger)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(m_instance, m_debugMessenger, nullptr);
    }
    if (m_instance)          vkDestroyInstance(m_instance, nullptr);
}

// ------------------------------------------------------------------
// pickPhysicalDevice - pilih GPU discrete, fallback ke GPU pertama
// ------------------------------------------------------------------
void Context::pickPhysicalDevice()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

    for (auto& dev : devices)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            m_physicalDevice = dev;
            LOG_INFO(("GPU: " + std::string(props.deviceName)).c_str());
            return;
        }
    }

    if (!devices.empty())
    {
        m_physicalDevice = devices[0];
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
        LOG_INFO(("GPU (fallback): " + std::string(props.deviceName)).c_str());
    }

    assert(m_physicalDevice != VK_NULL_HANDLE && "Tidak ada GPU Vulkan");
}

// ------------------------------------------------------------------
// createDevice - buat logical device dengan queue graphics
// ------------------------------------------------------------------
void Context::createDevice()
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, queues.data());

    for (uint32_t i = 0; i < count; i++)
    {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_graphicsIndex = i;
            break;
        }
    }
    assert(m_graphicsIndex != UINT32_MAX && "Tidak ada queue graphics");

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qi{};
    qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qi.queueFamilyIndex = m_graphicsIndex;
    qi.queueCount = 1;
    qi.pQueuePriorities = &priority;

    const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo di{};
    di.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    di.queueCreateInfoCount = 1;
    di.pQueueCreateInfos = &qi;
    di.enabledExtensionCount = 1;
    di.ppEnabledExtensionNames = devExts;
    di.pEnabledFeatures = &features;

    VkResult result = vkCreateDevice(m_physicalDevice, &di, nullptr, &m_device);
    assert(result == VK_SUCCESS && "Gagal buat device");

    vkGetDeviceQueue(m_device, m_graphicsIndex, 0, &m_graphicsQueue);
}

}

#include "renderer/Context.hpp"
#include "core/Window.hpp"
#include "core/Log.hpp"

#include <cstring>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#ifdef NDEBUG
static constexpr bool s_aktifkanValidation = false;
#else
static constexpr bool s_aktifkanValidation = true;
#endif

static const char* s_layerValidasi[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const char* s_ekstensiPerangkat[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Context::Context()
    : m_instance(VK_NULL_HANDLE)
    , m_surface(VK_NULL_HANDLE)
    , m_physicalDevice(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE)
    , m_graphicsQueue(VK_NULL_HANDLE)
    , m_presentQueue(VK_NULL_HANDLE)
    , m_graphicsFamily(0)
    , m_presentFamily(0)
    , m_allocator(VK_NULL_HANDLE)
    , m_window(nullptr)
{
}

Context::~Context() {
    cleanup();
}

bool Context::init(Window* window) {
    m_window = window;
    Log::info("Context menginisialisasi...");

    // --- Instance ---
    VkApplicationInfo infoApp{};
    infoApp.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    infoApp.pApplicationName = "Vulkana";
    infoApp.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    infoApp.pEngineName = "Vulkana Engine";
    infoApp.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    infoApp.apiVersion = VK_API_VERSION_1_3;

    uint32_t jumlahEkstensiGLFW = 0;
    const char** ekstensiGLFW = glfwGetRequiredInstanceExtensions(&jumlahEkstensiGLFW);

    VkInstanceCreateInfo infoBuat{};
    infoBuat.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    infoBuat.pApplicationInfo = &infoApp;
    infoBuat.enabledExtensionCount = jumlahEkstensiGLFW;
    infoBuat.ppEnabledExtensionNames = ekstensiGLFW;

    if (s_aktifkanValidation) {
        Log::info("Layer validasi diaktifkan");
        infoBuat.enabledLayerCount = 1;
        infoBuat.ppEnabledLayerNames = s_layerValidasi;
    }

    VkResult hasil = vkCreateInstance(&infoBuat, nullptr, &m_instance);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat instance Vulkan");
        return false;
    }
    Log::info("Instance Vulkan dibuat");

    // --- Surface (butuh window + instance) ---
    VkWin32SurfaceCreateInfoKHR infoPermukaan{};
    infoPermukaan.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    infoPermukaan.hwnd = glfwGetWin32Window(window->getHandle());
    infoPermukaan.hinstance = GetModuleHandle(nullptr);

    hasil = vkCreateWin32SurfaceKHR(m_instance, &infoPermukaan, nullptr, &m_surface);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat surface");
        return false;
    }
    Log::info("Surface dibuat di Context");

    // --- Pilih GPU (sekarang punya surface untuk cek present) ---
    if (!pickPhysicalDevice()) {
        Log::error("Tidak ada GPU yang sesuai");
        return false;
    }

    // --- Perangkat logis ---
    float prioritasQueue = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> infoQueueBuat;
    uint32_t familyUnik[] = { m_graphicsFamily, m_presentFamily };

    for (uint32_t family : familyUnik) {
        bool sudahAda = false;
        for (const auto& q : infoQueueBuat) {
            if (q.queueFamilyIndex == family) {
                sudahAda = true;
                break;
            }
        }
        if (!sudahAda) {
            VkDeviceQueueCreateInfo qci{};
            qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qci.queueFamilyIndex = family;
            qci.queueCount = 1;
            qci.pQueuePriorities = &prioritasQueue;
            infoQueueBuat.push_back(qci);
        }
    }

    VkDeviceCreateInfo infoPerangkat{};
    infoPerangkat.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    infoPerangkat.queueCreateInfoCount = static_cast<uint32_t>(infoQueueBuat.size());
    infoPerangkat.pQueueCreateInfos = infoQueueBuat.data();
    infoPerangkat.enabledExtensionCount = 1;
    infoPerangkat.ppEnabledExtensionNames = s_ekstensiPerangkat;

    VkPhysicalDeviceFeatures fiturPerangkat{};
    infoPerangkat.pEnabledFeatures = &fiturPerangkat;

    hasil = vkCreateDevice(m_physicalDevice, &infoPerangkat, nullptr, &m_device);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat perangkat logis");
        return false;
    }
    Log::info("Perangkat logis dibuat");

    vkGetDeviceQueue(m_device, m_graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentFamily, 0, &m_presentQueue);

    // --- VMA ---
    VmaAllocatorCreateInfo infoAlokator{};
    infoAlokator.vulkanApiVersion = VK_API_VERSION_1_3;
    infoAlokator.physicalDevice = m_physicalDevice;
    infoAlokator.device = m_device;
    infoAlokator.instance = m_instance;

    hasil = vmaCreateAllocator(&infoAlokator, &m_allocator);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat VMA allocator");
        return false;
    }
    Log::info("VMA allocator dibuat");

    return true;
}

void Context::cleanup() {
    if (m_allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

bool Context::isDeviceSuitable(VkPhysicalDevice device) {
    if (!findQueueFamilies(device)) return false;

    uint32_t jumlahEkstensi = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &jumlahEkstensi, nullptr);
    std::vector<VkExtensionProperties> ekstensiTersedia(jumlahEkstensi);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &jumlahEkstensi, ekstensiTersedia.data());

    for (const char* diminta : s_ekstensiPerangkat) {
        bool ditemukan = false;
        for (const auto& ext : ekstensiTersedia) {
            if (strcmp(diminta, ext.extensionName) == 0) {
                ditemukan = true;
                break;
            }
        }
        if (!ditemukan) return false;
    }

    return true;
}

bool Context::findQueueFamilies(VkPhysicalDevice device) {
    uint32_t jumlahFamily = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &jumlahFamily, nullptr);
    std::vector<VkQueueFamilyProperties> daftarFamily(jumlahFamily);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &jumlahFamily, daftarFamily.data());

    m_graphicsFamily = UINT32_MAX;
    m_presentFamily = UINT32_MAX;

    for (uint32_t i = 0; i < jumlahFamily; i++) {
        // Cari graphics
        if (daftarFamily[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphicsFamily = i;
        }
        // Cari present (butuh surface)
        if (m_surface != VK_NULL_HANDLE) {
            VkBool32 dukunganPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &dukunganPresent);
            if (dukunganPresent) {
                m_presentFamily = i;
            }
        }
    }

    // Fallback: jika present tidak ditemukan, pakai graphics
    if (m_presentFamily == UINT32_MAX) {
        m_presentFamily = m_graphicsFamily;
    }

    return m_graphicsFamily != UINT32_MAX;
}

bool Context::pickPhysicalDevice() {
    uint32_t jumlahPerangkat = 0;
    vkEnumeratePhysicalDevices(m_instance, &jumlahPerangkat, nullptr);
    if (jumlahPerangkat == 0) {
        Log::error("Tidak ada GPU yang mendukung Vulkan");
        return false;
    }

    std::vector<VkPhysicalDevice> perangkatTersedia(jumlahPerangkat);
    vkEnumeratePhysicalDevices(m_instance, &jumlahPerangkat, perangkatTersedia.data());

    int skorTerbaik = -1;
    VkPhysicalDevice perangkatTerbaik = VK_NULL_HANDLE;

    for (const auto& perangkat : perangkatTersedia) {
        if (!isDeviceSuitable(perangkat)) continue;

        VkPhysicalDeviceProperties properti;
        vkGetPhysicalDeviceProperties(perangkat, &properti);

        int skor = 0;
        if (properti.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            skor += 1000;
        }

        if (skor > skorTerbaik) {
            skorTerbaik = skor;
            perangkatTerbaik = perangkat;
        }
    }

    if (perangkatTerbaik == VK_NULL_HANDLE) {
        Log::error("Tidak ada GPU yang sesuai");
        return false;
    }

    m_physicalDevice = perangkatTerbaik;

    VkPhysicalDeviceProperties properti;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properti);
    Log::info("GPU terpilih: ");
    Log::info(properti.deviceName);

    return true;
}

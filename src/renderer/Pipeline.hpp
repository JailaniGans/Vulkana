#pragma once

// Pipeline - kompilasi shader + graphics pipeline

#include <volk.h>
#include <string_view>
#include <vector>
#include <cstdint>

namespace Vulkana {

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();

    void init(VkDevice device, VkRenderPass renderPass,
              std::string_view vertPath, std::string_view fragPath,
              VkExtent2D extent);
    void cleanup();

    VkPipeline handle() const { return m_pipeline; }
    VkPipelineLayout layout() const { return m_layout; }

private:
    VkShaderModule createModule(const std::vector<char>& code);
    std::vector<char> readFile(std::string_view path);

    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
};

}

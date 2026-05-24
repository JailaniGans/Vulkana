#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class Context;

/**
 * Pipeline - Pipeline grafis Vulkan dan pengelola shader.
 * Memuat SPIR-V dan membangun VkPipeline + VkPipelineLayout.
 */
class Pipeline {
public:
    Pipeline();
    ~Pipeline();

    bool create(Context* context,
                VkExtent2D extent,
                VkRenderPass renderPass,
                VkDescriptorSetLayout descriptorSetLayout);
    void destroy();

    VkPipeline       getPipeline() const { return m_pipeline; }
    VkPipelineLayout getLayout()   const { return m_pipelineLayout; }

private:
    Context*    m_context;
    VkPipeline  m_pipeline;
    VkPipelineLayout m_pipelineLayout;
    VkShaderModule m_modulVert;
    VkShaderModule m_modulFrag;

    VkShaderModule buatShaderModule(const std::string& namaFile);
};

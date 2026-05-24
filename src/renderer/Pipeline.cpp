#include "renderer/Pipeline.hpp"
#include "core/Log.hpp"
#include <fstream>
#include <cassert>

namespace Vulkana {

Pipeline::~Pipeline()
{
    cleanup();
}

// ------------------------------------------------------------------
// init - baca SPIR-V, buat shader module, pipeline layout, pipeline
// ------------------------------------------------------------------
void Pipeline::init(VkDevice device, VkRenderPass renderPass,
                    std::string_view vertPath, std::string_view fragPath,
                    VkExtent2D extent)
{
    m_device = device;

    auto vertCode = readFile(vertPath);
    auto fragCode = readFile(fragPath);
    assert(!vertCode.empty() && !fragCode.empty() && "Shader file kosong");

    VkShaderModule vertMod = createModule(vertCode);
    VkShaderModule fragMod = createModule(fragCode);

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertMod;
    stages[0].pName = "main";

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragMod;
    stages[1].pName = "main";

    // Vertex binding & attribute (pos vec3 + color vec3)
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(float) * 6;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[2]{};
    attrs[0].location = 0;
    attrs[0].binding = 0;
    attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[0].offset = 0;

    attrs[1].location = 1;
    attrs[1].binding = 0;
    attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[1].offset = sizeof(float) * 3;

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &binding;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.width  = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.pViewports = &viewport;
    vp.scissorCount = 1;
    vp.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                         | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &blend;

    // Push constant untuk MVP (3 mat4 = 144 byte)
    VkPushConstantRange pcRange{};
    pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pcRange.offset = 0;
    pcRange.size = sizeof(float) * 48; // 3 * 16 floats

    VkPipelineLayoutCreateInfo pl{};
    pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pl.pushConstantRangeCount = 1;
    pl.pPushConstantRanges = &pcRange;

    VkResult res = vkCreatePipelineLayout(m_device, &pl, nullptr, &m_layout);
    assert(res == VK_SUCCESS && "Gagal buat pipeline layout");

    VkGraphicsPipelineCreateInfo pi{};
    pi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pi.stageCount = 2;
    pi.pStages = stages;
    pi.pVertexInputState = &vi;
    pi.pInputAssemblyState = &ia;
    pi.pViewportState = &vp;
    pi.pRasterizationState = &rs;
    pi.pMultisampleState = &ms;
    pi.pDepthStencilState = &ds;
    pi.pColorBlendState = &cb;
    pi.layout = m_layout;
    pi.renderPass = renderPass;

    res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pi, nullptr, &m_pipeline);
    assert(res == VK_SUCCESS && "Gagal buat graphics pipeline");

    vkDestroyShaderModule(m_device, fragMod, nullptr);
    vkDestroyShaderModule(m_device, vertMod, nullptr);
    LOG_INFO("Pipeline siap");
}

void Pipeline::cleanup()
{
    if (m_pipeline) vkDestroyPipeline(m_device, m_pipeline, nullptr);
    if (m_layout)   vkDestroyPipelineLayout(m_device, m_layout, nullptr);
}

// ------------------------------------------------------------------
// createModule - buat VkShaderModule dari kode SPIR-V
// ------------------------------------------------------------------
VkShaderModule Pipeline::createModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule mod;
    VkResult res = vkCreateShaderModule(m_device, &ci, nullptr, &mod);
    assert(res == VK_SUCCESS && "Gagal buat shader module");
    return mod;
}

// ------------------------------------------------------------------
// readFile - baca file binary
// ------------------------------------------------------------------
std::vector<char> Pipeline::readFile(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    if (!file) return {};

    size_t size = (size_t)file.tellg();
    file.seekg(0);
    std::vector<char> buf(size);
    file.read(buf.data(), (std::streamsize)size);
    return buf;
}

}

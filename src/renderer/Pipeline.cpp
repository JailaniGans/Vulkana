#include "renderer/Pipeline.hpp"
#include "renderer/Context.hpp"
#include "core/Log.hpp"

#include <fstream>
#include <cassert>

Pipeline::Pipeline()
    : m_context(nullptr)
    , m_pipeline(VK_NULL_HANDLE)
    , m_pipelineLayout(VK_NULL_HANDLE)
    , m_modulVert(VK_NULL_HANDLE)
    , m_modulFrag(VK_NULL_HANDLE)
{
}

Pipeline::~Pipeline() {
    destroy();
}

bool Pipeline::create(Context* context,
                      VkExtent2D extent,
                      VkRenderPass renderPass,
                      VkDescriptorSetLayout descriptorSetLayout)
{
    m_context = context;
    VkDevice perangkat = context->getDevice();

    m_modulVert = buatShaderModule("build/scene.vert.spv");
    if (m_modulVert == VK_NULL_HANDLE) {
        Log::error("Gagal memuat vertex shader (build/scene.vert.spv)");
        return false;
    }

    m_modulFrag = buatShaderModule("build/scene.frag.spv");
    if (m_modulFrag == VK_NULL_HANDLE) {
        Log::error("Gagal memuat fragment shader (build/scene.frag.spv)");
        return false;
    }
    Log::info("Modul shader dimuat");

    VkPipelineLayoutCreateInfo infoLayout{};
    infoLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    infoLayout.setLayoutCount = 1;
    infoLayout.pSetLayouts = &descriptorSetLayout;
    infoLayout.pushConstantRangeCount = 0;

    VkResult hasil = vkCreatePipelineLayout(perangkat, &infoLayout, nullptr, &m_pipelineLayout);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat pipeline layout");
        return false;
    }

    VkPipelineShaderStageCreateInfo stageVert{};
    stageVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stageVert.module = m_modulVert;
    stageVert.pName = "main";

    VkPipelineShaderStageCreateInfo stageFrag{};
    stageFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stageFrag.module = m_modulFrag;
    stageFrag.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = { stageVert, stageFrag };

    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = 24;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrDesc[2]{};
    attrDesc[0].location = 0;
    attrDesc[0].binding = 0;
    attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrDesc[0].offset = 0;

    attrDesc[1].location = 1;
    attrDesc[1].binding = 0;
    attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrDesc[1].offset = 12;

    VkPipelineVertexInputStateCreateInfo inputVertex{};
    inputVertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputVertex.vertexBindingDescriptionCount = 1;
    inputVertex.pVertexBindingDescriptions = &bindingDesc;
    inputVertex.vertexAttributeDescriptionCount = 2;
    inputVertex.pVertexAttributeDescriptions = attrDesc;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState blendLampiran{};
    blendLampiran.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                   VK_COLOR_COMPONENT_G_BIT |
                                   VK_COLOR_COMPONENT_B_BIT |
                                   VK_COLOR_COMPONENT_A_BIT;
    blendLampiran.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &blendLampiran;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo infoPipeline{};
    infoPipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    infoPipeline.stageCount = 2;
    infoPipeline.pStages = stages;
    infoPipeline.pVertexInputState = &inputVertex;
    infoPipeline.pInputAssemblyState = &inputAssembly;
    infoPipeline.pViewportState = &viewportState;
    infoPipeline.pRasterizationState = &rasterizer;
    infoPipeline.pMultisampleState = &multisampling;
    infoPipeline.pDepthStencilState = &depthStencil;
    infoPipeline.pColorBlendState = &colorBlending;
    infoPipeline.pDynamicState = &dynamicState;
    infoPipeline.layout = m_pipelineLayout;
    infoPipeline.renderPass = renderPass;
    infoPipeline.subpass = 0;
    infoPipeline.basePipelineHandle = VK_NULL_HANDLE;

    hasil = vkCreateGraphicsPipelines(perangkat, VK_NULL_HANDLE, 1, &infoPipeline, nullptr, &m_pipeline);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat graphics pipeline");
        return false;
    }

    Log::info("Graphics pipeline dibuat");
    return true;
}

void Pipeline::destroy() {
    VkDevice perangkat = m_context ? m_context->getDevice() : VK_NULL_HANDLE;
    if (perangkat == VK_NULL_HANDLE) return;

    if (m_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(perangkat, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(perangkat, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }
    if (m_modulVert != VK_NULL_HANDLE) {
        vkDestroyShaderModule(perangkat, m_modulVert, nullptr);
        m_modulVert = VK_NULL_HANDLE;
    }
    if (m_modulFrag != VK_NULL_HANDLE) {
        vkDestroyShaderModule(perangkat, m_modulFrag, nullptr);
        m_modulFrag = VK_NULL_HANDLE;
    }
}

VkShaderModule Pipeline::buatShaderModule(const std::string& namaFile) {
    std::ifstream file(namaFile, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        Log::error("Gagal membuka file shader");
        Log::error(namaFile.c_str());
        return VK_NULL_HANDLE;
    }

    size_t ukuranFile = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(ukuranFile);
    file.seekg(0);
    file.read(buffer.data(), ukuranFile);
    file.close();

    VkShaderModuleCreateInfo infoBuat{};
    infoBuat.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    infoBuat.codeSize = buffer.size();
    infoBuat.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule modul;
    VkResult hasil = vkCreateShaderModule(m_context->getDevice(), &infoBuat, nullptr, &modul);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat shader module");
        return VK_NULL_HANDLE;
    }

    return modul;
}

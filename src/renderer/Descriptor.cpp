#include "renderer/Descriptor.hpp"
#include "renderer/Context.hpp"
#include "renderer/Buffer.hpp"
#include "core/Log.hpp"

Descriptor::Descriptor()
    : m_context(nullptr)
    , m_layout(VK_NULL_HANDLE)
    , m_pool(VK_NULL_HANDLE)
{
}

Descriptor::~Descriptor() {
    destroy();
}

bool Descriptor::create(Context* context, uint32_t maxFrames) {
    m_context = context;
    VkDevice perangkat = context->getDevice();

    VkDescriptorSetLayoutBinding bindingUBO{};
    bindingUBO.binding = 0;
    bindingUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindingUBO.descriptorCount = 1;
    bindingUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindingUBO.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo infoLayout{};
    infoLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    infoLayout.bindingCount = 1;
    infoLayout.pBindings = &bindingUBO;

    VkResult hasil = vkCreateDescriptorSetLayout(perangkat, &infoLayout, nullptr, &m_layout);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat layout descriptor set");
        return false;
    }
    Log::info("Layout descriptor set dibuat");

    VkDescriptorPoolSize ukuranPool{};
    ukuranPool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ukuranPool.descriptorCount = maxFrames;

    VkDescriptorPoolCreateInfo infoPool{};
    infoPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    infoPool.poolSizeCount = 1;
    infoPool.pPoolSizes = &ukuranPool;
    infoPool.maxSets = maxFrames;

    hasil = vkCreateDescriptorPool(perangkat, &infoPool, nullptr, &m_pool);
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal membuat descriptor pool");
        return false;
    }
    Log::info("Descriptor pool dibuat");

    return true;
}

void Descriptor::destroy() {
    VkDevice perangkat = m_context ? m_context->getDevice() : VK_NULL_HANDLE;
    if (perangkat == VK_NULL_HANDLE) return;

    m_sets.clear();

    if (m_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(perangkat, m_pool, nullptr);
        m_pool = VK_NULL_HANDLE;
    }
    if (m_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(perangkat, m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }
}

bool Descriptor::allocateSets(const std::vector<Buffer*>& uniformBuffers) {
    VkDevice perangkat = m_context->getDevice();
    uint32_t jumlah = static_cast<uint32_t>(uniformBuffers.size());

    std::vector<VkDescriptorSetLayout> layouts(jumlah, m_layout);

    VkDescriptorSetAllocateInfo infoAlokasi{};
    infoAlokasi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    infoAlokasi.descriptorPool = m_pool;
    infoAlokasi.descriptorSetCount = jumlah;
    infoAlokasi.pSetLayouts = layouts.data();

    m_sets.resize(jumlah);
    VkResult hasil = vkAllocateDescriptorSets(perangkat, &infoAlokasi, m_sets.data());
    if (hasil != VK_SUCCESS) {
        Log::error("Gagal mengalokasikan descriptor set");
        return false;
    }

    for (uint32_t i = 0; i < jumlah; i++) {
        VkDescriptorBufferInfo infoBuffer{};
        infoBuffer.buffer = uniformBuffers[i]->getBuffer();
        infoBuffer.offset = 0;
        infoBuffer.range = uniformBuffers[i]->getSize();

        VkWriteDescriptorSet writeDesc{};
        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = m_sets[i];
        writeDesc.dstBinding = 0;
        writeDesc.dstArrayElement = 0;
        writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDesc.descriptorCount = 1;
        writeDesc.pBufferInfo = &infoBuffer;
        writeDesc.pImageInfo = nullptr;
        writeDesc.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(perangkat, 1, &writeDesc, 0, nullptr);
    }

    Log::info("Descriptor set dialokasikan dan diperbarui");
    return true;
}

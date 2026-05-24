#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Context;
class Buffer;

/**
 * Descriptor - Pengelola descriptor set Vulkan.
 * Layout, pool, dan alokasi set untuk uniform buffer.
 */
class Descriptor {
public:
    Descriptor();
    ~Descriptor();

    bool create(Context* context, uint32_t maxFrames);
    void destroy();
    bool allocateSets(const std::vector<Buffer*>& uniformBuffers);

    VkDescriptorSetLayout getLayout() const { return m_layout; }
    VkDescriptorSet       getSet(uint32_t index) const { return m_sets[index]; }
    VkDescriptorPool      getPool()   const { return m_pool; }

private:
    Context*            m_context;
    VkDescriptorSetLayout m_layout;
    VkDescriptorPool    m_pool;
    std::vector<VkDescriptorSet> m_sets;
};

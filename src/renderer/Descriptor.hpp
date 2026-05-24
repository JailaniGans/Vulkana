#pragma once

// Descriptor - helper untuk layout, pool, alokasi, update descriptor set

#include <volk.h>
#include <vector>

namespace Vulkana {

class Descriptor {
public:
    static VkDescriptorSetLayout createLayout(
        VkDevice device,
        const std::vector<VkDescriptorSetLayoutBinding>& bindings);

    static VkDescriptorPool createPool(
        VkDevice device,
        uint32_t maxSets,
        const std::vector<VkDescriptorPoolSize>& poolSizes);

    static VkDescriptorSet allocate(
        VkDevice device,
        VkDescriptorPool pool,
        VkDescriptorSetLayout layout);

    static void update(
        VkDevice device,
        VkDescriptorSet set,
        const std::vector<VkWriteDescriptorSet>& writes);
};

}

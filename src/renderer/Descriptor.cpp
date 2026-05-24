#include "renderer/Descriptor.hpp"
#include <cassert>

namespace Vulkana {

// ------------------------------------------------------------------
// createLayout - buat VkDescriptorSetLayout dari binding array
// ------------------------------------------------------------------
VkDescriptorSetLayout Descriptor::createLayout(
    VkDevice device,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayoutCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount = (uint32_t)bindings.size();
    ci.pBindings = bindings.data();

    VkDescriptorSetLayout layout;
    VkResult res = vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);
    assert(res == VK_SUCCESS && "Gagal buat descriptor set layout");
    return layout;
}

// ------------------------------------------------------------------
// createPool - buat VkDescriptorPool
// ------------------------------------------------------------------
VkDescriptorPool Descriptor::createPool(
    VkDevice device,
    uint32_t maxSets,
    const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.maxSets = maxSets;
    ci.poolSizeCount = (uint32_t)poolSizes.size();
    ci.pPoolSizes = poolSizes.data();

    VkDescriptorPool pool;
    VkResult res = vkCreateDescriptorPool(device, &ci, nullptr, &pool);
    assert(res == VK_SUCCESS && "Gagal buat descriptor pool");
    return pool;
}

// ------------------------------------------------------------------
// allocate - alokasi satu descriptor set dari pool
// ------------------------------------------------------------------
VkDescriptorSet Descriptor::allocate(
    VkDevice device,
    VkDescriptorPool pool,
    VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool = pool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &layout;

    VkDescriptorSet set;
    VkResult res = vkAllocateDescriptorSets(device, &ai, &set);
    assert(res == VK_SUCCESS && "Gagal alokasi descriptor set");
    return set;
}

// ------------------------------------------------------------------
// update - update descriptor set dengan write descriptors
// ------------------------------------------------------------------
void Descriptor::update(
    VkDevice device,
    VkDescriptorSet set,
    const std::vector<VkWriteDescriptorSet>& writes)
{
    vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

}

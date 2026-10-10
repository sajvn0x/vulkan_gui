#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "vk_buffer.hh"

namespace vk {

struct VulkanDevice {
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkDevice logical_device{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceFeatures enabled_features{};
    VkPhysicalDeviceMemoryProperties memory_properties{};
    std::vector<VkQueueFamilyProperties> queue_family_properties{};
    std::vector<std::string> supported_extensions{};
    VkCommandPool command_pool{VK_NULL_HANDLE};

    struct {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queue_family_indices;

    operator VkDevice() const { return logical_device; }
    explicit VulkanDevice(VkPhysicalDevice physical_device);
    ~VulkanDevice();
    uint32_t get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties,
                             VkBool32 *mem_type_found = nullptr) const;
    uint32_t get_queue_family_index(VkQueueFlags queue_flags) const;
    VkResult create_logical_device(VkPhysicalDeviceFeatures enabled_features,
                                   std::vector<const char *> enabled_extensions, void *p_next_chain,
                                   bool use_swap_chain = true,
                                   VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    VkResult create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags,
                           VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
    VkResult create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, Buffer *buffer,
                           VkDeviceSize size, void *data = nullptr);
    void copy_buffer(Buffer *src, Buffer *dst, VkQueue queue, VkBufferCopy *copy_region = nullptr);
    VkCommandPool create_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags =
                                                                       VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
    VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, bool begin = false);
    void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool pool, bool free = true);
    void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, bool free = true);
    bool extension_supported(std::string extension);
    VkFormat get_supported_depth_format(bool check_sampling_support);
};

};  // namespace vk

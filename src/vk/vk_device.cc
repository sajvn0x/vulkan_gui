#include "vk_device.hh"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "vk_initializers.hh"
#include "vulkan/vulkan_core.h"

namespace vk {

VulkanDevice::VulkanDevice(VkPhysicalDevice physical_device) {
    assert(physical_device);
    this->physical_device = physical_device;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    assert(queue_family_count > 0);
    queue_family_properties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

    // list of supported extension
    uint32_t ext_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, nullptr);
    if (ext_count > 0) {
        std::vector<VkExtensionProperties> extensions(ext_count);
        if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, &extensions.front()) ==
            VK_SUCCESS) {
            for (auto &ext : extensions) {
                supported_extensions.push_back(ext.extensionName);
            }
        }
    }
}

VulkanDevice::~VulkanDevice() {
    if (command_pool) {
        vkDestroyCommandPool(logical_device, command_pool, nullptr);
    }
    if (logical_device) {
        vkDestroyDevice(logical_device, nullptr);
    }
}

uint32_t VulkanDevice::get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties,
                                       VkBool32 *mem_type_found) const {
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if ((type_bits & 1) == 1) {
            if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                if (mem_type_found) {
                    *mem_type_found = true;
                }
                return i;
            }
        }
        type_bits >>= 1;
    }

    if (mem_type_found) {
        *mem_type_found = false;
        return 0;
    } else {
        throw std::runtime_error("could not find a matching memory type");
    }

    return 0;
}

uint32_t VulkanDevice::get_queue_family_index(VkQueueFlags queue_flags) const {
    if ((queue_flags & VK_QUEUE_COMPUTE_BIT) == queue_flags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); ++i) {
            if ((queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    if ((queue_flags & VK_QUEUE_TRANSFER_BIT) == queue_flags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++) {
            if ((queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    throw std::runtime_error("could not find a matching queue family index");
}

VkResult VulkanDevice::create_logical_device(VkPhysicalDeviceFeatures enabled_features,
                                             std::vector<const char *> enabled_extensions, void *p_next_chain,
                                             bool use_swap_chain, VkQueueFlags requested_queue_types) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};

    const float default_queue_priority(0.0f);

    if (requested_queue_types & VK_QUEUE_GRAPHICS_BIT) {
        queue_family_indices.graphics = get_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = queue_family_indices.graphics;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &default_queue_priority;
        queue_create_infos.push_back(queue_info);
    } else {
        queue_family_indices.graphics = 0;
    }

    if (requested_queue_types & VK_QUEUE_COMPUTE_BIT) {
        queue_family_indices.graphics = get_queue_family_index(VK_QUEUE_COMPUTE_BIT);
        if (queue_family_indices.compute != queue_family_indices.graphics) {
            VkDeviceQueueCreateInfo queue_info = {};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = queue_family_indices.compute;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &default_queue_priority;
            queue_create_infos.push_back(queue_info);
        }
    } else {
        queue_family_indices.compute = queue_family_indices.graphics;
    }

    if (requested_queue_types & VK_QUEUE_TRANSFER_BIT) {
        queue_family_indices.graphics = get_queue_family_index(VK_QUEUE_TRANSFER_BIT);
        if ((queue_family_indices.transfer != queue_family_indices.graphics) &&
            (queue_family_indices.transfer != queue_family_indices.compute)) {
            VkDeviceQueueCreateInfo queue_info = {};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = queue_family_indices.transfer;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &default_queue_priority;
            queue_create_infos.push_back(queue_info);
        }
    } else {
        queue_family_indices.compute = queue_family_indices.graphics;
    }

    std::vector<const char *> device_extensions(enabled_extensions);
    if (use_swap_chain) {
        device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_info.pQueueCreateInfos = queue_create_infos.data();
    device_info.pEnabledFeatures = &enabled_features;

    VkPhysicalDeviceFeatures2 physical_device_features2 = {};
    if (p_next_chain) {
        physical_device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physical_device_features2.features = enabled_features;
        physical_device_features2.pNext = p_next_chain;
        device_info.pEnabledFeatures = nullptr;
        device_info.pNext = &physical_device_features2;
    }

    if (device_extensions.size() > 0) {
        for (const char *enabled_extension : device_extensions) {
            if (!extension_supported(enabled_extension)) {
                std::cerr << "enabled device extension \"" << enabled_extension
                          << "\" is not present at device level\n";
            }
        }

        device_info.enabledExtensionCount = (uint32_t)device_extensions.size();
        device_info.ppEnabledExtensionNames = device_extensions.data();
    }

    this->enabled_features = enabled_features;

    VkResult result = vkCreateDevice(physical_device, &device_info, nullptr, &logical_device);
    if (result != VK_SUCCESS) {
        return result;
    }

    command_pool = create_command_pool(queue_family_indices.graphics);

    return result;
}

VkResult VulkanDevice::create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags,
                                     VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data) {
    VkResult result = VK_SUCCESS;
    VkBufferCreateInfo buffer_info = vk::initializers::buffer_create_info(usage_flags, size);
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    result = vkCreateBuffer(logical_device, &buffer_info, nullptr, buffer);
    if (result != VK_SUCCESS) return result;

    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc = vk::initializers::memory_allocate_info();
    vkGetBufferMemoryRequirements(logical_device, *buffer, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = get_memory_type(mem_reqs.memoryTypeBits, memory_property_flags);

    VkMemoryAllocateFlagsInfoKHR alloc_flags_info{};
    if (usage_flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        alloc_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        mem_alloc.pNext = &alloc_flags_info;
    }

    result = vkAllocateMemory(logical_device, &mem_alloc, nullptr, memory);
    if (result != VK_SUCCESS) return result;

    if (data != nullptr) {
        void *mapped;
        result = vkMapMemory(logical_device, *memory, 0, size, 9, &mapped);
        memcpy(mapped, data, size);
        if ((memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mapped_range{};
            mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mapped_range.memory = *memory;
            mapped_range.size = size;

            vkFlushMappedMemoryRanges(logical_device, 1, &mapped_range);
        };
        vkUnmapMemory(logical_device, *memory);
    }

    result = vkBindBufferMemory(logical_device, *buffer, *memory, 0);
    if (result != VK_SUCCESS) return result;

    return result;
}

VkResult VulkanDevice::create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags,
                                     Buffer *buffer, VkDeviceSize size, void *data) {
    VkResult result = VK_SUCCESS;
    buffer->device = logical_device;

    VkBufferCreateInfo buffer_info = vk::initializers::buffer_create_info(usage_flags, size);
    result = vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer->buffer);
    if (result != VK_SUCCESS) return result;

    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc = vk::initializers::memory_allocate_info();
    vkGetBufferMemoryRequirements(logical_device, buffer->buffer, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = get_memory_type(mem_reqs.memoryTypeBits, memory_property_flags);

    VkMemoryAllocateFlagsInfoKHR alloc_flags_info{};
    if (usage_flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        alloc_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        mem_alloc.pNext = &alloc_flags_info;
    }
    result = vkAllocateMemory(logical_device, &mem_alloc, nullptr, &buffer->memory);
    if (result != VK_SUCCESS) return result;

    buffer->alignment = mem_reqs.alignment;
    buffer->size = size;
    buffer->usage_flags = usage_flags;
    buffer->memory_property_flags = memory_property_flags;

    if (data != nullptr) {
        result = buffer->map();
        if (result != VK_SUCCESS) return result;
        memcpy(buffer->mapped, data, size);
        if ((memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) buffer->flush();

        buffer->unmap();
    }

    buffer->setup_descriptor();
    return buffer->bind();
}

void VulkanDevice::copy_buffer(Buffer *src, Buffer *dst, VkQueue queue, VkBufferCopy *copy_region) {
    assert(dst->size >= src->size);
    assert(src->buffer);
    VkCommandBuffer copy_cmd = create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy buffer_copy{};
    if (copy_region == nullptr) {
        buffer_copy.size = src->size;
    } else {
        buffer_copy = *copy_region;
    }

    vkCmdCopyBuffer(copy_cmd, src->buffer, dst->buffer, 1, &buffer_copy);
    flush_command_buffer(copy_cmd, queue);
}

VkCommandPool VulkanDevice::create_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.flags = create_flags;
    cmd_pool_info.queueFamilyIndex = queue_family_index;

    VkCommandPool cmd_pool;
    // TODO: handle errors properly
    vkCreateCommandPool(logical_device, &cmd_pool_info, nullptr, &cmd_pool);
    return cmd_pool;
}

VkCommandBuffer VulkanDevice::create_command_buffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin) {
    VkCommandBufferAllocateInfo cmd_buf_allocate_info = vk::initializers::command_buffer_allocate_info(pool, level, 1);
    VkCommandBuffer cmd_buffer;
    vkAllocateCommandBuffers(logical_device, &cmd_buf_allocate_info, &cmd_buffer);
    if (begin) {
        VkCommandBufferBeginInfo cmd_buf_info = vk::initializers::command_buffer_begin_info();
        vkBeginCommandBuffer(cmd_buffer, &cmd_buf_info);
    }
    return cmd_buffer;
}

VkCommandBuffer VulkanDevice::create_command_buffer(VkCommandBufferLevel level, bool begin) {
    return create_command_buffer(level, command_pool, begin);
}

void VulkanDevice::flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool pool, bool free) {
#define DEFAULT_FENCE_TIMEOUT 100000000000
    VkResult result = VK_SUCCESS;

    if (command_buffer == VK_NULL_HANDLE) {
        return;
    }

    result = vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    VkFenceCreateInfo fence_info = vk::initializers::fence_create_info(0);
    VkFence fence;
    result = vkCreateFence(logical_device, &fence_info, nullptr, &fence);
    result = vkQueueSubmit(queue, 1, &submit_info, fence);
    result = vkWaitForFences(logical_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
    vkDestroyFence(logical_device, fence, nullptr);

    if (free) {
        vkFreeCommandBuffers(logical_device, pool, 1, &command_buffer);
    }
}

void VulkanDevice::flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, bool free) {
    return flush_command_buffer(command_buffer, queue, command_pool, free);
}

bool VulkanDevice::extension_supported(std::string extension) {
    return (std::find(supported_extensions.begin(), supported_extensions.end(), extension) !=
            supported_extensions.end());
}

VkFormat VulkanDevice::get_supported_depth_format(bool check_sampling_support) {
    std::vector<VkFormat> depth_formats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
                                           VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                           VK_FORMAT_D16_UNORM};

    for (auto &format : depth_formats) {
        VkFormatProperties format_properties;

        if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            if (check_sampling_support) {
                if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
                    continue;
                }
            }
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

};  // namespace vk

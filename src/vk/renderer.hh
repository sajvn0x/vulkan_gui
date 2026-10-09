#pragma once

#include <vulkan/vulkan.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "core/error.hh"

class VulkanRenderer {
    static VulkanRenderer* singleton;

    std::string name = "vulkan gui";

public:
    static VulkanRenderer* get_singleton() { return singleton; }

    VulkanRenderer();
    ~VulkanRenderer();

private:
    VkInstance instance{VK_NULL_HANDLE};
    // should support version 1.4
    uint32_t vk_api_version = VK_VERSION_1_4;
    std::map<std::string, bool> requested_instance_extensions;
    std::set<std::string> enabled_instance_extension_names;
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties device_properties{};
    VkPhysicalDeviceFeatures device_features{};
    VkPhysicalDeviceMemoryProperties device_memory_properties{};
    VkPhysicalDeviceFeatures enabled_features;
    std::vector<const char*> enabled_device_extensions;
    std::vector<const char*> enabled_instance_extensions;
    std::vector<VkLayerSettingEXT> enabled_layer_settings;
    void* device_create_p_next_chain = nullptr;
    VkDevice vk_device{VK_NULL_HANDLE};
    VkQueue vk_queue{VK_NULL_HANDLE};

    Error _initialize_vulkan_version();
    void _register_requested_instance_extension(const std::string& p_extension_name, bool p_required);
    Error _initialize_instance();

public:
    Error initialize();
    // void setup_depth_stencil();
    // void setup_framebuffer();
    // void setup_render_pass();
    // void get_enabled_features();
    // void get_enabled_extensions();
    // void begin_dynamic_rendering(VkCommandBuffer cmd_buffer);
    // void end_dynamic_rendering(VkCommandBuffer cmd_buffer);

    VkPipelineShaderStageCreateInfo load_shader(std::string filename, VkShaderStageFlagBits stage);
};

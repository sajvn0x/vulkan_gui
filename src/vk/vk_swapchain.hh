#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vk {

class VulkanSwapchain {
private:
    VkInstance instance{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

public:
    VkFormat color_format{};
    VkColorSpaceKHR color_space{};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<VkImage> images{};
    std::vector<VkImageView> image_views{};
    uint32_t queue_node_index{UINT32_MAX};
    uint32_t image_count{0};

    void init_surface();
    void set_context(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device);
    void create(uint32_t& width, uint32_t& height, bool vsync = false, bool fullscreen = false);
    VkResult acquire_next_image(VkSemaphore present_complete_semaphore, uint32_t& image_index);
    void cleanup();
};

};  // namespace vk

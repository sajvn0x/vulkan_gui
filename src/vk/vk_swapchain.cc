#include "vk_swapchain.hh"

#include <cassert>

#include "core/window_management.hh"

namespace vk {

void VulkanSwapchain::init_surface() { WindowManagement::get_singleton()->vulkan_get_surface(instance, &surface); }

void VulkanSwapchain::set_context(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device) {
    this->instance = instance;
    this->physical_device = physical_device;
    this->device = device;
}

void VulkanSwapchain::create(uint32_t& width, uint32_t& height, bool vsync, bool fullscreen) {
    assert(physical_device);
    assert(device);
    assert(instance);

    VkSwapchainKHR old_swapchain = nullptr;

    // physical device surface properties and formats
    VkSurfaceCapabilitiesKHR surface_caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_caps);

    VkExtent2D swapchain_extent = {};
    if (surface_caps.currentExtent.width == (uint32_t)-1) {
        swapchain_extent.width = width;
        swapchain_extent.height = height;
    } else {
        swapchain_extent = surface_caps.currentExtent;
        width = surface_caps.currentExtent.width;
        height = surface_caps.currentExtent.height;
    }

    // prensent mode
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    assert(present_mode_count > 0);

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);

    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if (!vsync) {
        for (size_t i = 0; i < present_mode_count; ++i) {
            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                break;
            }
        }
    }

    uint32_t desired_number_of_swapchain_images = surface_caps.minImageCount + 1;
    if ((surface_caps.maxImageCount > 0) && (desired_number_of_swapchain_images > surface_caps.maxImageCount)) {
        desired_number_of_swapchain_images = surface_caps.minImageCount;
    }

    VkSurfaceTransformFlagsKHR pre_transform;
    if (surface_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        pre_transform = surface_caps.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    for (auto& composite_alpha_flag : composite_alpha_flags) {
        if (surface_caps.supportedCompositeAlpha & composite_alpha_flag) {
            composite_alpha = composite_alpha_flag;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_ci = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                             .surface = surface,
                                             .minImageCount = desired_number_of_swapchain_images,
                                             .imageFormat = color_format,
                                             .imageExtent = {swapchain_extent.width, swapchain_extent.height},
                                             .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                             .queueFamilyIndexCount = 0,
                                             .preTransform = (VkSurfaceTransformFlagBitsKHR)pre_transform,
                                             .compositeAlpha = composite_alpha,
                                             .presentMode = swapchain_present_mode,
                                             .clipped = VK_TRUE,
                                             .oldSwapchain = old_swapchain};

    if (surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (old_swapchain != VK_NULL_HANDLE) {
        for (auto i = 0; i < images.size(); i++) {
            vkDestroyImageView(device, image_views[i], nullptr);
        }
        vkDestroySwapchainKHR(device, old_swapchain, nullptr);
    }

    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, images.data());

    image_views.resize(image_count);
    for (auto i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo color_attachment_view{};
        color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_attachment_view.image = images[i];
        color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_attachment_view.format = color_format;
        color_attachment_view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                            VK_COMPONENT_SWIZZLE_A};
        color_attachment_view.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                  .baseMipLevel = 0,
                                                  .levelCount = 1,
                                                  .baseArrayLayer = 0,
                                                  .layerCount = 1};
        vkCreateImageView(device, &color_attachment_view, nullptr, &image_views[i]);
    }
}

VkResult VulkanSwapchain::acquire_next_image(VkSemaphore present_complete_semaphore, uint32_t& image_index) {
    return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, present_complete_semaphore, (VkFence) nullptr,
                                 &image_index);
}

void VulkanSwapchain::cleanup() {
    if (swapchain != VK_NULL_HANDLE) {
        for (auto i = 0; i < images.size(); ++i) {
            vkDestroyImageView(device, image_views[i], nullptr);
        }
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    surface = VK_NULL_HANDLE;
    swapchain = VK_NULL_HANDLE;
}

}  // namespace vk

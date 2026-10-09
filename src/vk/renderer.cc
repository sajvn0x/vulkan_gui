#include "renderer.hh"

#include "core/window_management.hh"

Error VulkanRenderer::initialize() {
    Error err = OK;

    err = _initialize_vulkan_version();
    if (err != OK) return err;

    err = _initialize_instance();
    if (err != OK) return err;

    return err;
}

Error VulkanRenderer::_initialize_vulkan_version() {
    VkResult result = vkEnumerateInstanceVersion(&vk_api_version);
    if (result != VK_SUCCESS || vk_api_version < VK_API_VERSION_1_2) return ERR_CANT_CREATE;
    return OK;
}

void VulkanRenderer::_register_requested_instance_extension(const std::string& p_extension_name, bool p_required) {
    requested_instance_extensions[p_extension_name] = p_required;
}

Error VulkanRenderer::_initialize_instance() {
    enabled_instance_extension_names.clear();

    std::vector<const char*> extensions = WindowManagement::get_singleton()->vulkan_get_platform_extensions();
    for (const char*& extension : extensions) {
        _register_requested_instance_extension(extension, true);
    }

    _register_requested_instance_extension(VK_KHR_SURFACE_EXTENSION_NAME, true);
    _register_requested_instance_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, false);
    _register_requested_instance_extension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, false);
    _register_requested_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false);

    uint32_t instance_extension_count = 0;
    VkResult err = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
    ERR_FAIL_COND_V(err != VK_SUCCESS && err != VK_INCOMPLETE, ERR_CANT_CREATE);
    ERR_FAIL_COND_V(instance_extension_count == 0, ERR_CANT_CREATE);

    std::vector<VkExtensionProperties> instance_extensions(instance_extension_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data());
    if (err != VK_SUCCESS && err != VK_INCOMPLETE) {
        return ERR_CANT_CREATE;
    }

    for (uint32_t i = 0; i < instance_extension_count; i++) {
        std::string extension_name = instance_extensions[i].extensionName;

        if (requested_instance_extensions.find(extension_name) != requested_instance_extensions.end()) {
            enabled_instance_extension_names.insert(extension_name);
        }
    }

    for (auto& requested_extension : requested_instance_extensions) {
        const std::string& name = requested_extension.first;
        bool required = requested_extension.second;

        if (enabled_instance_extension_names.find(name) == enabled_instance_extension_names.end()) {
            if (required) {
                return ERR_CANT_CREATE;
            }
        }
    }

    VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                               .pApplicationName = name.c_str(),
                               .pEngineName = name.c_str(),
                               .apiVersion = vk_api_version};

    std::vector<const char*> enabled_layer_names;
    enabled_layer_names.push_back("VK_LAYER_KHRONOS_validation");

    std::vector<const char*> enabled_extension_names;
    enabled_extension_names.reserve(enabled_instance_extension_names.size());
    for (const std::string& ext_name : enabled_instance_extension_names) {
        enabled_extension_names.push_back(ext_name.data());
    }

    VkInstanceCreateInfo instance_info = {};
    instance_info.pApplicationInfo = &app_info;
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.enabledExtensionCount = enabled_extension_names.size();
    instance_info.ppEnabledExtensionNames = enabled_extension_names.data();
    instance_info.enabledLayerCount = enabled_layer_names.size();
    instance_info.ppEnabledLayerNames = enabled_layer_names.data();

    VkResult res = vkCreateInstance(&instance_info, nullptr, &instance);
    ERR_FAIL_COND_V(res != VK_SUCCESS, ERR_CANT_CREATE);

    return OK;
}

VulkanRenderer* VulkanRenderer::singleton = nullptr;

VulkanRenderer::VulkanRenderer() { singleton = this; }

VulkanRenderer::~VulkanRenderer() {
    if (singleton) {
        singleton = nullptr;
    }
}

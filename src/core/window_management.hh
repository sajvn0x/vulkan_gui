#pragma once

#include <string>
#include <vector>

#if defined(VULKAN_ENABLED)
#include <vulkan/vulkan.h>
#endif  // VULKAN_ENABLED

typedef int WindowId;

enum { INVALID_WINDOW_ID = -1 };

class WindowManagement {
    static WindowManagement *singleton;

public:
    static WindowManagement *get_singleton() { return singleton; }

public:
    WindowManagement();
    virtual ~WindowManagement();
    WindowManagement(const WindowManagement &) = delete;
    WindowManagement &operator=(const WindowManagement &) = delete;

public:
    virtual std::string get_name() const = 0;
    virtual void process_events() = 0;

    // screen
public:
    virtual int get_screen_count() const = 0;

    // window
public:
    virtual std::vector<WindowId> get_window_list() = 0;
    virtual void show_window(WindowId p_id) = 0;
    virtual void window_set_title(const std::string p_title) = 0;

// vulkan
#if defined(VULKAN_ENABLED)
public:
    virtual std::vector<const char *> vulkan_get_platform_extensions() const = 0;
    virtual VkResult vulkan_get_surface(VkInstance p_instance, VkSurfaceKHR *r_surface) const = 0;
#endif
};

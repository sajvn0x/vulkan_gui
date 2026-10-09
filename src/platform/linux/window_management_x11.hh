#pragma once

#include <X11/Xlib.h>

#include <map>

#if defined(VULKAN_ENABLED)
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#endif

#include "core/window_management.hh"

enum { PRIMARY_WINDOW = 0 };

class WindowManagementX11 : public WindowManagement {
    ::Display* display = nullptr;

    // window
private:
    WindowId window_id_counter = 0;

    struct WindowData {
        Window window;

        // size and position
        int width;
        int height;
        int x_pos;
        int y_pos;

        bool app_focused = true;
        Window embed_parent = 0;
        bool fullscreen = false;
        bool resize_disabled = false;
    };
    std::map<int, WindowData> windows;

    bool xinerama_ext_ok = false;
    bool xrandr_ext_ok = false;

    WindowId _create_window(std::string p_title, int p_width, int p_height, int p_x_pos, int p_y_pos,
                            Window p_parent_window);
    void _show_window(WindowId p_wid);

public:
    std::vector<WindowId> get_window_list() override;
    void show_window(WindowId p_id) override;
    void window_set_title(const std::string p_title) override;

public:
    std::string get_name() const override;
    void process_events() override;

    int get_screen_count() const override;

#if defined(VULKAN_ENABLED)
public:
    std::vector<const char*> vulkan_get_platform_extensions() const override;
    VkResult vulkan_get_surface(VkInstance p_instance, VkSurfaceKHR* r_surface) const override;
#endif

    WindowManagementX11();
    ~WindowManagementX11() override;
};

#include "window_management_x11.hh"

#include <X11/XKBlib.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>

#include <iostream>

std::string WindowManagementX11::get_name() const { return "X11"; }

void WindowManagementX11::process_events() {
    XEvent event;
    bool loop = true;

    while (loop) {
        XNextEvent(display, &event);

        switch (event.type) {
            case Expose: {
                for (int i = 0; i < window_id_counter; ++i) {
                    if (windows[i].window == event.xkey.window) {
                        windows[i].app_focused = true;
                    } else {
                        windows[i].app_focused = false;
                    }
                }
            } break;

            case KeyPress: {
                KeySym keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
                if (keysym == XK_Escape) loop = false;
                break;
            }
        }
    }
}

int WindowManagementX11::get_screen_count() const {
    int monitor_count = 0;
    if (xinerama_ext_ok) {
        XineramaQueryScreens(display, &monitor_count);
    }

    if (monitor_count == 0) {
        monitor_count = XScreenCount(display);
    }

    return monitor_count;
}

std::vector<WindowId> WindowManagementX11::get_window_list() {
    std::vector<WindowId> wids;
    for (const auto& [key, value] : windows) {
        wids.push_back(key);
    }

    return wids;
}

void WindowManagementX11::show_window(WindowId p_id) { _show_window(p_id); }

void WindowManagementX11::window_set_title(const std::string p_title) {
    if (!p_title.empty()) {
        XStoreName(display, windows[0].window, p_title.c_str());
    }
}

WindowId WindowManagementX11::_create_window(std::string p_title, int p_width, int p_height, int p_x_pos, int p_y_pos,
                                             Window p_parent_window) {
    int screen = DefaultScreen(display);
    XSetWindowAttributes attributes = {.background_pixel = BlackPixel(display, screen),
                                       .event_mask = ExposureMask | KeyPressMask};

    Window window =
        XCreateWindow(display, p_parent_window, p_x_pos, p_y_pos, p_width, p_height, 0, DefaultDepth(display, screen),
                      InputOutput, DefaultVisual(display, screen), CWBackPixel | CWEventMask, &attributes);

    if (!p_title.empty()) {
        XStoreName(display, window, p_title.c_str());
    }

    WindowId wid = window_id_counter++;
    windows[wid].window = window;
    windows[wid].embed_parent = p_parent_window;
    windows[wid].width = p_width;
    windows[wid].height = p_height;
    windows[wid].x_pos = p_x_pos;
    windows[wid].y_pos = p_y_pos;

    return wid;
}

void WindowManagementX11::_show_window(WindowId p_wid) { XMapWindow(display, windows[p_wid].window); }

WindowManagementX11::WindowManagementX11() {
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "failed to open x11 dispaly" << std::endl;
        return;
    }

    // xinerama extension
    {
        int event_base;
        int error_base;

        Bool supported = XineramaQueryExtension(display, &event_base, &error_base);
        xinerama_ext_ok = supported;
    }

    // xrandr extension
    {
        int event_base;
        int error_base;

        Bool supported = XRRQueryExtension(display, &event_base, &error_base);
        xrandr_ext_ok = supported;
    }

    WindowId wid = _create_window("vulkan gui", 1920, 1080, 0, 0, DefaultRootWindow(display));
    _show_window(wid);
}

WindowManagementX11::~WindowManagementX11() {
    for (int i = 0; i < window_id_counter; ++i) {
        if (windows[i].window) {
            XUnmapWindow(display, windows[i].window);
            XDestroyWindow(display, windows[i].window);
        }
    }
    XCloseDisplay(display);
}

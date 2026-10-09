#pragma once

#include <string>
#include <vector>

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
};

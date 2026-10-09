#include <iostream>

#include "app.hh"
#include "core/window_management.hh"
#include "platform/linux/window_management_x11.hh"

WindowManagement* ws = nullptr;

int main() {
    // window management
#if defined(PLATFORM_LINUX)
    ws = new WindowManagementX11();
#elif defined(PLATFORM_WINDOWS)
#endif

    if (!ws) {
        std::cout << "failed to initialize window management" << std::endl;
        return -1;
    }

    int error_code = 0;
    App* app = new App(&error_code);
    if (error_code) return error_code;

    app->run();

    return 0;
}

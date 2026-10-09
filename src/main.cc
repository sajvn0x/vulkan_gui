#include <iostream>

#include "core/window_management.hh"
#include "platform/linux/window_management_x11.hh"

WindowManagement *ws = nullptr;

int main() {
    // window management
    ws = new WindowManagementX11();

    if (!ws) {
        std::cout << "failed to initialize window management" << std::endl;
        return -1;
    }

    ws->process_events();  // loop
}

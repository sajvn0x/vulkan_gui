#include "app.hh"

#include "core/window_management.hh"
#include "vk/renderer.hh"

App::App(int *r_code) {
    // ensure singleton classes are initialized
    if (!WindowManagement::get_singleton()) {
        *r_code = -1;
        return;
    }
    if (!VulkanRenderer::get_singleton()) {
        *r_code = -1;
        return;
    }

    *r_code = 0;
}

App::~App() {}

void App::run() {
    WindowManagement::get_singleton()->process_events();  // main loop
}

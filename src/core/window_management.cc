#include "window_management.hh"

WindowManagement *WindowManagement::singleton = nullptr;

WindowManagement::WindowManagement() { singleton = this; }

WindowManagement::~WindowManagement() {
    if (singleton) {
        singleton = nullptr;
    }
}

#pragma once

class App {
public:
    App(int* r_code);
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

private:
    int last_fps;
};

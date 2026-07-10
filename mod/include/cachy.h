#pragma once
#include "process.h"
#include "reversed.h"

#include <fstream>

#define LOG(LVL, ...) RS.log_stream << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl

class CachyRS
{
public:
    ProcessInterface pi;

public:
    std::ofstream log_stream;

public:
    FnEglSwapBuffers real_eglSwapBuffers;
    FnSDL_PollEvent real_SDL_PollEvent;

private:
    void init_logging();
    void init_process_info();
    void init_imgui();

public:
    void flush_logs();

public:
    void hook_import(const std::string &symbol, void **original, void *target);

public:
    Globals *get_globals();

public:
    bool project_to_screen(Vec3<float> scene, Vec2<float> *out);

public:
    void init();
};

extern CachyRS RS;
#pragma once
#include "process.h"
#include "reversed.h"
#include "ui.h"
#include "ring_buffer.h"
#include "interop.h"
#include "hook.h"
#include "version.hpp"

#include <fstream>
#include <mutex>
#include <atomic>

#include <stdarg.h>

#define FEATURE_VERSION "0.0.9"

#define LOG(LVL, ...)                                                                                     \
    RS.log_mutex.lock();                                                                                  \
    RS.log_stream << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
    RS.flush_logs();                                                                                      \
    RS.log_mutex.unlock()

class CachyRS
{
public:
    ProcessInterface pi;
    HookManager* hook_manager;
    
public:
    std::ofstream log_stream;
    std::mutex log_mutex;

public:
    EglSwapBuffersHook hook_egl_swap_buffers;
    SdlPollEventHook hook_sdl_poll_event;
    MenuExecuteHook hook_menu_execute;

public:
    UserInterface* ui = nullptr;
    RingBuffer<SDL_Event> event_ring_buffer;

private:
    void init_logging();
    void init_process_info();
    void init_imgui();

public:
    void flush_logs();

public:
    std::string get_configuration_dir();
    std::string resolve_configuration(const std::string &file);

public:

public:
    Globals *get_globals();

public:
    bool project_to_screen(Vec3<float> scene, Vec2<float> *out);

public:
    void init();
};

extern CachyRS RS;
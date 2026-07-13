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

#define FEATURE_VERSION "0.0.11"

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
    std::unique_ptr<UserInterface> ui = nullptr;
    RingBuffer<SDL_Event> event_ring_buffer;

private:
    void init_logging();
    void init_process_info();
    void init_imgui();

public:
    void flush_logs();

public:
    std::string get_configuration_dir() const;
    std::string resolve_configuration(const std::string &file) const;

public:

public:
    Globals *get_globals() const;

public:
    bool project_to_screen(const Vec3<float> scene, Vec2<float> *out) const;

public:
    void init();
};

extern CachyRS RS;
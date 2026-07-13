#pragma once
#include "process.h"

#include "ui.h"

#include "reversed/reversed.h"
#include "hook.h"

#include "ring_buffer.h"
#include "util.h"
#include "interop.h"

#include "version.hpp"

#include <fstream>
#include <mutex>
#include <atomic>

#include <stdarg.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 14
#define VERSION_API 0

// clang-format off
#define FEATURE_VERSION                \
    MACRO_TO_STRING(VERSION_MAJOR)     \
    "." MACRO_TO_STRING(VERSION_MINOR) \
    "." MACRO_TO_STRING(VERSION_PATCH) \
    "." MACRO_TO_STRING(VERSION_API)
// clang-format on

#define LOG(LVL, ...)                                                                                     \
    RS.log_mutex.lock();                                                                                  \
    RS.log_stream << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
    RS.flush_logs();                                                                                      \
    RS.log_mutex.unlock()

class CachyRS
{
public:
    ProcessInterface pi;
    std::unique_ptr<HookManager> hook_manager = nullptr;

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
    Globals *get_globals() const;

public:
    bool project_to_screen(const Vec3<float> scene, Vec2<float> *out) const;

public:
    void init();
};

extern CachyRS RS;
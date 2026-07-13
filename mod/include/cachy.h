#pragma once
#include "process.h"

#include "ui.h"

#include "reversed/reversed.h"
#include "hook.h"

#include "ring_buffer.h"
#include "util.h"
#include "log.h"
#include "interop.h"

#include "version.hpp"

#include <fstream>
#include <mutex>
#include <atomic>

#include <stdarg.h>

namespace crs
{
    class CachyRS
    {
    public:
        ProcessInterface pi;
        ::std::unique_ptr<HookManager> hook_manager = nullptr;

    public:
        ::std::unique_ptr<UserInterface> ui = nullptr;
        RingBuffer<SDL_Event> event_ring_buffer;

    private:
        void init_logging();
        void init_process_info();
        void init_imgui();

    public:
        ::std::string get_configuration_dir() const;
        ::std::string resolve_configuration(const ::std::string &file) const;

    public:
        Globals *get_globals() const;

    public:
        bool project_to_screen(const Vec3<float> scene, Vec2<float> *out) const;

    public:
        void init();
    };

    extern CachyRS RS;
}
#pragma once
#include "process.h"
#include "reversed.h"
#include "cachy_rmlui.h"
#include "ring_buffer.h"
#include "interop.h"
#include "version.hpp"

#include <fstream>
#include <mutex>
#include <atomic>

#include <stdarg.h>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

#define FEATURE_VERSION "0.0.7"

#define LOG(LVL, ...)                                                                                     \
    RS.log_mutex.lock();                                                                                  \
    RS.log_stream << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
    RS.flush_logs();                                                                                      \
    RS.log_mutex.unlock()

#define off(t, f) offsetof(t, f)

template <typename T, typename... A>
FINLINE T dref(void *obj, std::initializer_list<uint64_t> args)
{
    void *last = obj;
    for (auto a : args)
    {
        last = *((void **)((char *)last + a));
    }

    return (T)last;
}

class CachyRS
{
public:
    ProcessInterface pi;

public:
    std::ofstream log_stream;
    std::mutex log_mutex;

public:
    FnEglSwapBuffers real_eglSwapBuffers;
    FnSDL_PollEvent real_SDL_PollEvent;
    RingBuffer<SDL_Event> event_ring_buffer;
    bool rmlui_wants_input = false;

public:
    CachySystemInterface rmlui_system_interface;
    Rml::Context *rmlui_context = nullptr;
    Rml::ElementDocument *rmlui_root_document = nullptr;

    Rml::Element *rmlui_selected_tab_button = nullptr;
    Rml::Element *rmlui_selected_content = nullptr;

    Rml::Element *rmlui_home_tab_button = nullptr;
    Rml::Element *rmlui_home_content = nullptr;

    Rml::Element *rmlui_plugins_tab_button = nullptr;
    Rml::Element *rmlui_plugins_content = nullptr;

    Rml::Element *rmlui_debug_tab_button = nullptr;
    Rml::Element *rmlui_debug_content = nullptr;
    Rml::Element *rmlui_debug_projection = nullptr;

public:
    bool player_overlay_on = false;

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
    void load_fonts();
    void reload_ui();
    void init_rmlui(int window_width, int window_height);
    void update_rmlui();

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
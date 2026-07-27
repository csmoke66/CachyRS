#pragma once
#include "hook.h"
#include "reversed/reversed.h"
#include "math.h"

#include <SDL2/SDL_syswm.h>
#include <dlfcn.h>

namespace crs
{
    class MenuExecuteHook : public Hook<FnMenuExecute>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class EglSwapBuffersHook : public Hook<FnEglSwapBuffers>
    {
    public:
        EGLint cached_width, cached_height;
        bool is_first_run = true;

    private:
        void render_widget(const Engine *engine, const RenderWidgetHook *rw_hook, const SdlPollEventHook *spe_hook, const Widget *widget, int x, int y);

    public:
        void handler(CpuState *cpu_state) override;
    };

    class EglGetDisplayHook : public Hook<FnEglGetDisplay>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class EglInitHook : public Hook<FnEglInit>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class EglCreateWindowSurfaceHook : public Hook<FnEglCreateWindowSurface>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class EglChooseConfigHook : public Hook<FnEglChooseConfig>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class SdlGetWindowWMInfoHook : public Hook<FnSdlGetWindowWMInfo>
    {
    public:
        SDL_SysWMinfo info;

    public:
        void handler(CpuState *cpu_state) override;
    };

    class SdlPollEventHook : public Hook<FnSDL_PollEvent>
    {
    public:
        Vec2<float> mouse_pos;

    public:
        void handler(CpuState *cpu_state) override;
    };

    class SdlCreateWindowHook : public Hook<FnSDL_CreateWindow>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class SdlCreateContextHook : public Hook<FnSDL_CreateContext>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class SdlDestroyContextHook : public Hook<FnSDL_DeleteContext>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class DlOpenHook : public Hook<decltype(dlopen)*>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };

    class EngineTickHook : public Hook<FnEngineTick>
    {
    private:
        void tick_ui(Engine *engine);
        void tick_imgui(Engine *engine);
        void tick_stats();

    public:
        void handler(CpuState *cpu_state) override;
    };

    struct RenderedWidget
    {
        const Widget *widget;
        uint32_t time;
        uint32_t absolute_x;
        uint32_t absolute_y;
    };

    class RenderWidgetHook : public Hook<FnRenderWidget>
    {
    public:
        std::map<const Widget *, RenderedWidget> rendered;

    public:
        void handler(CpuState *cpu_state) override;
    };

    class SetVarBitHook : public Hook<FnSetVarBit>
    {
    public:
        void handler(CpuState *cpu_state) override;
    };
}
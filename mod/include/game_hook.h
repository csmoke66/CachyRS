#pragma once
#include "hook.h"
#include "reversed/reversed.h"
#include "math.h"

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

    class SdlPollEventHook : public Hook<FnSDL_PollEvent>
    {
    public:
        Vec2<float> mouse_pos;

    public:
        void handler(CpuState *cpu_state) override;
    };

    class EngineTickHook : public Hook<FnEngineTick>
    {
    private:
        void tick_ui(Engine* engine);
        void tick_imgui(Engine* engine);
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
#pragma once
#include "hook.h"
#include "math.h"
#include "reversed/reversed.h"

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

  class SdlPollEventHook : public Hook<FnSdlPollEvent>
  {
  public:
    Vec2<float> mouse_pos;

  public:
    void handler(CpuState *cpu_state) override;
  };

  class SdlCreateWindowHook : public Hook<FnSdlCreateWindow>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };

  class SdlCreateContextHook : public Hook<FnSdlCreateContext>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };

  class SdlDestroyContextHook : public Hook<FnSdlDeleteContext>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };

  class DlOpenHook : public Hook<decltype(dlopen) *>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };

  class EngineTickHook : public Hook<FnEngineTick>
  {
  private:
    struct ItemContainerCache
    {
      std::vector<Item> items;
    };

  private:
    bool plugins_loaded = false;
    std::map<uint32_t, ItemContainerCache> cached_containers;

  private:
    void tick_ui(Engine *engine);
    void tick_imgui(Engine *engine);
    void tick_stats();

  private:
    void watch_item_changes(Engine *engine);
    void watch_widget_changes(Engine *engine);

  public:
    void handler(CpuState *cpu_state) override;
  };

  class AddMenuOptionHook : public Hook<FnAddMenuOption>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };

  class AddChatMessageHook : public Hook<FnAddChatMessage>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };

  struct RenderedWidgetSnapshot
  {
    Widget *widget;
    uint32_t time;
  };

  class RenderWidgetHook : public Hook<FnRenderWidget>
  {
  private:
    std::map<const Widget *, RenderedWidgetSnapshot> snapshots;

  public:
    void handler(CpuState *cpu_state) override;

  public:
    bool is_visible(const Widget *w) const;
    void remove_stale(uint32_t before);
  };

  class SetVarBitHook : public Hook<FnSetVarBit>
  {
  public:
    void handler(CpuState *cpu_state) override;
  };
} // namespace crs
#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GL/gl.h>
#include <dlfcn.h>

#include <X11/Xlib.h>

#include <map>

namespace crs
{
  struct BlockedApi
  {
    void *real_function;
    void *new_function;

    template <typename T>
    T cast_function()
    {
      return (T)real_function;
    }
  };

  static std::vector<std::string> blocked = {
    "glBufferStorage",
    "glBufferData",
    "glNamedBufferStorage",
    "glNamedBufferData",
    "glCopyTexImage1D",
    "glCopyTexImage2D",
    "glTextureStorage1D",
    "glTextureStorage2D",
    "glTextureStorage3D",
    "glRenderbufferStorage",
    "glNamedRenderbufferStorage",
    "glRenderbufferStorageMultisample",
    "glNamedRenderbufferStorageMultisample",
    "glDrawElements",
    "glDrawArrays",
    "glDrawBuffer",
    "glDrawElementsBaseVertex",
    "glDrawArraysInstanced",
    "glDrawElementsInstanced",
    "glEnable",
    "glDisable",
    "glCopyTexSubImage2D",
    "glReadPixels",
    "glBlitFramebuffer",
    "glBufferSubData",
    "glTexImage2D",
    "glTexSubImage2D"
  };

  static void *dummy_hook()
  {
    return nullptr;
  }

  void EglGetProcAddressHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);
    auto name = reinterpret_cast<const char *>(CPU_FIRST_ARG(cpu_state));
    for (auto &s : blocked)
    {
      if (!s.compare(name))
      {
        cpu_state->rax = reinterpret_cast<uint64_t>(dummy_hook);
        return;
      }
    }

    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(name));
  }
} // namespace crs

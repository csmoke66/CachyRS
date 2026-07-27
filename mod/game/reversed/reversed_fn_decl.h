#pragma once

#include "reversed_fwd_decl.h"
 
#include <SDL2/SDL.h>

typedef EGLBoolean (*FnEglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
typedef EGLDisplay (*FnEglGetDisplay)(NativeDisplayType native_display);
typedef EGLDisplay (*FnEGlGetPlatformDispaly)(uint32_t, void*, void*);
typedef EGLBoolean (*FnEglInit)(EGLDisplay, EGLint*, EGLint*);
typedef EGLSurface (*FnEglCreateWindowSurface)(EGLDisplay display, EGLConfig config, NativeWindowType native_window, EGLint* attrib_list);
typedef EGLBoolean (*FnEglChooseConfig)(EGLDisplay display, EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
typedef SDL_bool (*FnSdlGetWindowWMInfo)(SDL_Window*, SDL_SysWMinfo*);
typedef int (*FnSDL_PollEvent)(SDL_Event* event);
typedef SDL_Window* (*FnSDL_CreateWindow)(const char*, int, int, int, int, uint32_t);
typedef SDL_GLContext (*FnSDL_CreateContext)(SDL_Window*);
typedef void (*FnSDL_DeleteContext)(SDL_GLContext);
typedef void* (*FnMenuExecute)(void* menu_context, const ActionMenuContext* context, void*);
typedef void* (*FnHeapAllocate)(void* heap, size_t size, size_t alignment);
typedef void* (*FnHeapAllocateAligned)(size_t size);
typedef void* (*FnEngineTick)(Engine* engine, float delta);
typedef void* (*FnRenderWidget)(Widget* widget, void*, JArray<WidgetChild>* widget_list, int c_x, int c_y, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void* (*FnSetVarBit)(const WorldSettingCache* cache, const CacheBuffer<void, WorldSettingMask> *buffer, const uint32_t* value);
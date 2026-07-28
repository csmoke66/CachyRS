#pragma once

#include "reversed_fwd_decl.h"
 
#include <SDL2/SDL.h>

typedef EGLBoolean (*FnEglSwapBuffers)(EGLDisplay, EGLSurface);
typedef EGLDisplay (*FnEglGetDisplay)(NativeDisplayType);
typedef EGLDisplay (*FnEGlGetPlatformDispaly)(uint32_t, void*, void*);
typedef EGLBoolean (*FnEglInit)(EGLDisplay, EGLint*, EGLint*);
typedef EGLSurface (*FnEglCreateWindowSurface)(EGLDisplay, EGLConfig, NativeWindowType, EGLint*);
typedef EGLBoolean (*FnEglChooseConfig)(EGLDisplay, EGLint*, EGLConfig*, EGLint, EGLint*);

typedef SDL_bool (*FnSdlGetWindowWMInfo)(SDL_Window*, SDL_SysWMinfo*);
typedef int (*FnSdlPollEvent)(SDL_Event* event);
typedef SDL_Window* (*FnSdlCreateWindow)(const char*, int, int, int, int, uint32_t);
typedef SDL_GLContext (*FnSdlCreateContext)(SDL_Window*);
typedef void (*FnSdlDeleteContext)(SDL_GLContext);

typedef void* (*FnMenuExecute)(void*, const ActionMenuContext*, void*);
typedef void* (*FnHeapAllocate)(void*, size_t, size_t);
typedef void* (*FnHeapAllocateAligned)(size_t);
typedef void* (*FnEngineTick)(Engine*, float);
typedef void* (*FnRenderWidget)(Widget*, void*, JArray<WidgetChild>*, int, int, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void* (*FnSetVarBit)(const WorldSettingCache*, const CacheBuffer<void, WorldSettingMask>*, const uint32_t*);
typedef void (*FnAddMenuOption)(void*, const char*, uint8_t*, int32_t, void*, int32_t*, int32_t, int32_t, int32_t, int32_t, uint8_t, uint8_t, int32_t, uint8_t, uint8_t, void*, uint8_t, int32_t);
#pragma once

#include "reversed_fwd_decl.h"

typedef EGLBoolean (*FnEglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
typedef int (*FnSDL_PollEvent)(SDL_Event* event);
typedef void* (*FnMenuExecute)(void* menu_context, const ActionMenuContext* context, void*);
typedef void* (*FnHeapAllocate)(void* heap, size_t size, size_t alignment);
typedef void* (*FnHeapAllocateAligned)(size_t size);
typedef void* (*FnEngineTick)(Engine* engine, float delta);
typedef void* (*FnRenderWidget)(Widget* widget, void*, JArray<WidgetChild>* widget_list, int c_x, int c_y, void*, void*, void*, void*, void*, void*, void*, void*);
#pragma once

#include "reversed_fwd_decl.h"

typedef EGLBoolean (*FnEglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
typedef int (*FnSDL_PollEvent)(SDL_Event* event);
typedef void* (*FnMenuExecute)(void* menu_context, const ActionMenuContext* context, void* unk_003);
typedef void* (*FnHeapAllocate)(void* heap, size_t size, size_t alignment);
typedef void* (*FnHeapAllocateAligned)(size_t size);

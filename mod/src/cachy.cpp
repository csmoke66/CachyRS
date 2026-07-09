#include "cachy.h"

#include <unistd.h>

#include <sys/mman.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <EGL/egl.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>

CachyRS RS;

uint64_t hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    static EGLint cached_width, cached_height;
    static bool is_first_run = true;

    ImGuiIO &io = ImGui::GetIO();

    if (is_first_run)
    {
        ImGui_ImplOpenGL3_Init("#version 330"); // TODO FIXME check error
        is_first_run = false;
    }

    EGLint width, height;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    LOG(DEBUG, "Original: " << std::hex << (uint64_t)RS.real_eglSwapBuffers);
    LOG(DEBUG, "Width: " << width << ", Height: " << height);
    if (cached_width != width || cached_height != height)
    {
        cached_width = width;
        cached_height = height;

        io.DisplaySize = ImVec2(width, height);
        io.DeltaTime = 1.0f / 60.0f;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Overlay");
    ImGui::Text("Hello from ImGui!");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return RS.real_eglSwapBuffers(dpy, surface);
}

void CachyRS::init_logging()
{
    log_stream = std::ofstream("/tmp/cachy-rs.txt");
}

void CachyRS::init_process_info()
{
    pi.init();
}

void CachyRS::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
}

void CachyRS::flush_logs()
{
    log_stream.flush();
}

void CachyRS::hook_import(const std::string &symbol, void **original, void *target)
{
    ImportedFunction fn;
    if (pi.find_import(symbol, &fn))
    {
        auto page = (Elf64_Addr)fn.addr & ~(getpagesize() - 1);

        mprotect((void *)page, getpagesize(), PROT_READ | PROT_WRITE);
        if (original)
        {
            *original = (void *)*fn.addr;
        }
        *fn.addr = (Elf64_Addr)target;
        mprotect((void *)page, getpagesize(), PROT_READ);
    }
}

void CachyRS::init()
{
    init_logging();

    LOG(INFO, "Initializing process info...");
    init_process_info();

    LOG(INFO, "Initializing ImGui...");
    init_imgui();

    LOG(INFO, "Placing hooks...");
    hook_import("eglSwapBuffers", (void **)&RS.real_eglSwapBuffers, (void *)&hook_eglSwapBuffers);
}

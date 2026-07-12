#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <GL/gl.h>

#define FLUSH_GL_ERRORS() while (glGetError() != GL_NO_ERROR) {}

template <typename FN>
void iterate_entities(const WorldNode *node, FN fn)
{
    if (auto entity = node->entity)
    {
        fn(entity);
    }

    for (auto c = node->children.begin; c != node->children.end; c++)
    {
        iterate_entities(*c, fn);
    }
}

void EglSwapBuffersHook::handler(CpuState *cpu_state)
{
    auto dpy = (EGLDisplay)CPU_FIRST_ARG(cpu_state);
    auto surface = (EGLSurface)CPU_SECOND_ARG(cpu_state);

    EGLint width, height;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (is_first_run)
    {
        ImGui_ImplOpenGL3_Init("#version 330"); // TODO FIXME check error
        RS.ui->init(std::string(FEATURE_VERSION) + CACHYRS_VERSION, RS.get_configuration_dir(), NRS.sdl_window(), width, height);

        // flush OpenGL errors so they don't propagate to rmlui
        FLUSH_GL_ERRORS();

        is_first_run = false;
    }

    if (cached_width != width || cached_height != height)
    {
        cached_width = width;
        cached_height = height;

        auto &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
        io.DeltaTime = 1.0f / 60.0f;
    }

    // clang-format off
    RS.event_ring_buffer.process([](auto& event)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        RS.ui->process(&event);
    });
    // clang-format on

    { /* imgui */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        if (ImGui::Begin("Developer"))
        {
            if (ImGui::Button("Reload UI"))
            {
                RS.ui->reload();
            }
        }

        if (RS.ui->get_bool(UserVariable::player_overlay))
        {
            if (auto root = NRS.root_node())
            {
                // clang-format off
                iterate_entities(root, [](const Entity *entity) 
                {
                    if (entity->type == EntityType::player)
                    {
                        Vec2<float> screen_pos;
                        if (RS.project_to_screen(entity->position, &screen_pos))
                        {
                            auto bg = ImGui::GetBackgroundDrawList();
                            bg->AddText(ImVec2(screen_pos.x, screen_pos.y), IM_COL32(255, 0, 0, 170), entity->name.c_str());
                        }
                    }
                });
                // clang-format on
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // flush OpenGL errors so they don't propagate to rmlui
    FLUSH_GL_ERRORS();

    { /* ui */
        UserInterfaceState state;
        if (auto scene = NRS.scene_003())
        {
            state.projection_matrix = scene->projection_matrix;
        }
        if (auto item_cache = NRS.item_cache())
        {
            auto& containers = item_cache->containers;
            for (auto i = containers.begin; i != containers.end; i++)
            {
                UserInterfaceItemContainer ui_container;
                ui_container.id = i->id;
                for (auto j = i->items.begin; j < i->items.end; j++)
                {
                    UserInterfaceItem ui_item;
                    ui_item.id = j->id;
                    ui_item.amount = j->amount;
                    if (ui_item.id != -1 && ui_item.amount > 0)
                    {
                        ui_container.items.push_back(ui_item);
                    }
                }
                state.item_containers.push_back(ui_container);
            }
        }

        RS.ui->propagate(state);
        RS.ui->render();
    }

    cpu_state->rax = (uint64_t)trampoline(dpy, surface);
}

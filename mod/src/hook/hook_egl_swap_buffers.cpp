#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <GL/gl.h>

#define FLUSH_GL_ERRORS()               \
    while (glGetError() != GL_NO_ERROR) \
    {                                   \
    }

namespace crs
{
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

    void EglSwapBuffersHook::render_widget(const Engine *engine, const RenderWidgetHook *rw_hook, const SdlPollEventHook* spe_hook, const Widget *widget, int x, int y)
    {
        auto bg = ImGui::GetBackgroundDrawList();
        auto rendered = rw_hook->rendered.find(widget);
        if (rendered != rw_hook->rendered.end() &&
            engine->time - rendered->second.time < 10)
        {
            if (widget->get_type() == WidgetType::container)
            {
                auto container = (ContainerWidget*)widget;
                for (auto c = container->children.begin; c != container->children.end; c++)
                {
                    if (auto cw = c->widget)
                    {
                        render_widget(engine, rw_hook, spe_hook, cw, container->x + x, container->y + y);
                    }
                }
            }
            else
            {
                auto a_x = rendered->second.absolute_x;
                auto a_y = rendered->second.absolute_y;

                auto min = ImVec2(a_x, a_y);
                auto max = ImVec2(min.x + widget->width, min.y + widget->height);

                auto& mp = spe_hook->mouse_pos;
                if (mp.x >= min.x && mp.x < max.x && mp.y >= min.y && mp.y < max.y)
                {
                    bg->AddRectFilled(min, max, IM_COL32(0, 255, 0, 20));
                }
                else
                {
                    bg->AddRectFilled(min, max, IM_COL32(255, 0, 0, 10));
                }
            }
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

            auto bg = ImGui::GetBackgroundDrawList();
            if (RS.ui->get_bool(UserVariable::player_overlay))
            {
                if (auto root = NRS.root_node())
                {
                    // clang-format off
                    iterate_entities(root, [&bg](const Entity *entity) 
                    {
                        if (entity->type == EntityType::player)
                        {
                            Vec2<float> screen_pos;
                            if (RS.project_to_screen(entity->position, &screen_pos))
                            {
                                bg->AddText(ImVec2(screen_pos.x, screen_pos.y), IM_COL32(255, 0, 0, 170), entity->name.c_str());
                            }
                        }
                    });
                    // clang-format on
                }
            }

            auto rw_hook = RS.hook_manager->view_hook<const RenderWidgetHook>("render_widget");
            auto spe_hook = RS.hook_manager->view_hook<const SdlPollEventHook>("poll_event");
            auto engine = RS.get_globals()->engine;
            auto widget_cache = NRS.widget_cache();
            if (engine && widget_cache)
            {
                for (auto widget_001 = widget_cache->c.begin;
                     widget_001 != widget_cache->c.end;
                     widget_001++)
                {
                    if (auto widget_002 = widget_001->widget_002)
                    {
                        for (auto widget_003 = widget_002->widgets_003.begin;
                             widget_003 != widget_002->widgets_003.end;
                             widget_003++)
                        {
                            if (auto widget = widget_003->widget)
                            {
                                render_widget(engine, rw_hook, spe_hook, widget, 0, 0);
                            }
                        }
                    }
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
                auto &containers = item_cache->containers;
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
}
#include "plugin.h"
#include "event_bus.h"
#include <format>
#include <plugin_cpp.h>

using namespace crs;

static std::shared_ptr<ApiDropDown> ui_mode_dropdown;
static ApiCheckBox ui_checkbox_override_walk;
static ApiCheckBox ui_checkbox_override_widgets;
static ApiCheckBox ui_checkbox_override_objects;
static ApiCheckBox ui_checkbox_override_players;
static ApiCheckBox ui_checkbox_override_npcs;

static ApiContainer ui_container_test;
static ApiContainer *ui_container_last_visible = nullptr;

static MenuActionTemplate our_template;

enum relax_mode : int32_t
{
    relax_mode_test
};

std::string Boot::name()
{
    return "Relax Mode";
}

static void override_menu_action(MenuActionEventArgs *event_args, void *handler, const std::array<uint32_t, 4> &args)
{
    memcpy(&our_template, *event_args->action_template, sizeof(MenuActionTemplate));
    our_template.handler = (FnMenuActionHandler)handler;
    *event_args->action_template = &our_template;

    memcpy(event_args->args->r, args.data(), sizeof(int32_t) * 4);
    event_args->bypass_logic = true;
}

void Boot::init()
{
    // clang-format off
    Api::on_menu_action([](MenuActionEventArgs* args)
    {
        auto tmpl = *args->action_template;
        auto wants_override = [tmpl]()
        {
            if (tmpl->type == MenuActionType::walk && ui_checkbox_override_walk.is_checked())
            {
                return true;
            }
            else if (tmpl->type == MenuActionType::widget && ui_checkbox_override_widgets.is_checked())
            {
                return true;
            }
            else if (tmpl->type == MenuActionType::obj && ui_checkbox_override_objects.is_checked())
            {
                return true;
            }
            else if (tmpl->type == MenuActionType::player && ui_checkbox_override_players.is_checked())
            {
                return true;
            }
            else if (tmpl->type == MenuActionType::npc && ui_checkbox_override_npcs.is_checked())
            {
                return true;
            }
            else
            {
                return false;
            }
        };

        auto override_action_test = [args]()
        {
            override_menu_action(args, &Api::raw_globals()->menu_action_handler_walk, { 0, 0xd13, 0xc7c, 0x6d200001 });
        };

        if (wants_override())
        {
            if (ui_mode_dropdown->is_selected(relax_mode_test))
            {
                override_action_test();
            }
        }
    });
    // clang-format on
}

void Boot::init_ui()
{
    Api::add_label("Relax Mode");
    Api::add_hr();
    ui_mode_dropdown = Api::add_dropdown({"Test"});

    // clang-format off
    ui_mode_dropdown->on_changed([](auto idx) 
    {
        if (ui_container_last_visible)
        {
            ui_container_last_visible->set_visible(false);
            ui_container_last_visible = nullptr;
        }

        switch (idx)
        {
        case relax_mode_test:
            ui_container_last_visible = &ui_container_test;
            break;
        }

        if (ui_container_last_visible)
        {
            ui_container_last_visible->set_visible(true);
        }
    });
    // clang-format on

    ui_checkbox_override_walk = Api::add_checkbox("Override Walking");
    ui_checkbox_override_widgets = Api::add_checkbox("Override Widgets");
    ui_checkbox_override_objects = Api::add_checkbox("Override Objects");
    ui_checkbox_override_players = Api::add_checkbox("Override Players");
    ui_checkbox_override_npcs = Api::add_checkbox("Override NPCs");

    ui_container_test = Api::add_container();
}

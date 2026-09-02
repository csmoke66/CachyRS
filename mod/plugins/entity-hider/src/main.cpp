#include "plugin.h"
#include "event_bus.h"
#include <format>
#include <plugin_cpp.h>

using namespace crs;

static ApiCheckBox ui_players_hide_checkbox;
static ApiCheckBox ui_players_show_self_checkbox;
static ApiCheckBox ui_players_show_friends_checkbox;
static ApiCheckBox ui_npcs_hide_checkbox;

std::string Boot::name()
{
    return "Entity Hider";
}

void Boot::init()
{
    // clang-format off
    Api::on_tick([]()
    {
        auto self = Api::raw_self();
        for (auto player : Api::raw_players())
        {
            if (auto node = player->parent)
            {
                auto hidden = ui_players_hide_checkbox.is_checked();
                if (hidden)
                {
                    if (ui_players_show_self_checkbox.is_checked())
                    {
                        if (player == self)
                        {
                            hidden = false;
                        }
                    }

                    if (ui_players_show_friends_checkbox.is_checked())
                    {
                        if (Api::raw_is_friend(player))
                        {
                            hidden = false;
                        }
                    }
                }

                if (hidden)
                {
                    node->flags &= ~WorldNodeFlag::has_entity;
                }
                else
                {
                    node->flags |= WorldNodeFlag::has_entity;
                }
            }
        }
    });
    // clang-format on
}

void Boot::init_ui()
{
    Api::add_label("Players");
    Api::add_hr();
    ui_players_hide_checkbox = Api::add_checkbox("Hidden");
    ui_players_show_self_checkbox = Api::add_checkbox("Show Self");
    ui_players_show_friends_checkbox = Api::add_checkbox("Show Friends");

    Api::add_label("NPCs");
    Api::add_hr();
    ui_npcs_hide_checkbox = Api::add_checkbox("Hidden");
}

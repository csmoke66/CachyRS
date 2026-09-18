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

std::string Boot::version()
{
  return "1.0.0";
}

void Boot::init()
{
  Api::on_tick([]()
  {
    const auto hide_players = ui_players_hide_checkbox.is_checked();
    const auto show_self = ui_players_show_self_checkbox.is_checked();
    const auto show_friends = ui_players_show_friends_checkbox.is_checked();
    const auto hide_npcs = ui_npcs_hide_checkbox.is_checked();

    for (auto &player : Api::players())
    {
      auto hidden = hide_players;
      if (hidden)
      {
        if (show_self && player.is_self())
        {
          hidden = false;
        }

        if (show_friends && player.is_friend())
        {
          hidden = false;
        }
      }

      player.set_rendered(!hidden);
    }

    for (auto &npc : Api::npcs())
    {
      npc.set_rendered(!hide_npcs);
    }
  });
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

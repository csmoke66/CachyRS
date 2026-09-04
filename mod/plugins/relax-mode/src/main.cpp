#include "archaeology.h"
#include "relax_mode.h"

using namespace crs;

static std::unique_ptr<DropDownContentChanger> ui_mode_content_changer;

static ApiCheckBox ui_checkbox_override_walk;
static ApiCheckBox ui_checkbox_override_widgets;
static ApiCheckBox ui_checkbox_override_objects;
static ApiCheckBox ui_checkbox_override_players;
static ApiCheckBox ui_checkbox_override_npcs;

static ApiContainer ui_container_test;

static ApiContainer ui_container_archaeology;
static std::unique_ptr<DropDownContentChanger> ui_archaeology_site_dropdown_content_changer;
static ApiContainer ui_archaeology_site_kharid_et;
static std::shared_ptr<ApiDropDown> ui_archaeology_site_kharid_et_spot;

static MenuActionTemplate our_template;

std::string Boot::name()
{
  return "Relax Mode";
}

void Boot::init()
{
  Api::on_menu_action([](MenuActionEventArgs *args)
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

    if (wants_override())
    {
      if (ui_mode_content_changer->is_selected(relax_mode_test))
      {
        Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::walk),
            { 0, 0xd13, 0xc7c, 0x6d200001 });
      }
      else if (ui_mode_content_changer->is_selected(relax_mode_archaeology))
      {
        if (ui_archaeology_site_dropdown_content_changer->is_selected(arch_site_kharid_et))
        {
          archaeology_override(args,
              ui_archaeology_site_dropdown_content_changer->get_selected(),
              ui_archaeology_site_kharid_et_spot->get_selected());
        }
      }
    }
  });
}

void Boot::init_ui()
{
  Api::add_label("Relax Mode");
  Api::add_hr();

  ui_mode_content_changer = DropDownContentChanger::new_changer(
      Api::root_plugin_component_id(), { { "Test", &ui_container_test },
                                           { "Archaeology", &ui_container_archaeology } });

  ui_checkbox_override_walk = Api::add_checkbox("Override Walking");
  ui_checkbox_override_widgets = Api::add_checkbox("Override Widgets");
  ui_checkbox_override_objects = Api::add_checkbox("Override Objects");
  ui_checkbox_override_players = Api::add_checkbox("Override Players");
  ui_checkbox_override_npcs = Api::add_checkbox("Override NPCs");

  ui_container_test = Api::add_container();
  ui_container_archaeology = Api::add_container();
  ui_mode_content_changer->reset();

  ui_archaeology_site_dropdown_content_changer = DropDownContentChanger::new_changer(
      ui_container_archaeology.get_id(), { { "Kharid-et", &ui_archaeology_site_kharid_et } });

  ui_archaeology_site_kharid_et = ui_container_archaeology.add_container();
  ui_archaeology_site_kharid_et_spot = ui_archaeology_site_kharid_et.add_dropdown({ "Entrace" });
  ui_archaeology_site_dropdown_content_changer->reset();
}

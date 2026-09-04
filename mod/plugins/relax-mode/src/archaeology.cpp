#include "archaeology.h"

static void khard_et_main_entrance_override(MenuActionEventArgs *args)
{
  auto self = Api::self();
  auto inventory = Api::get_inventory();
  auto soil_box = inventory.first([](auto &item)
  {
    return item.get_id() == 49538;
  });

  auto has_soil = inventory.contains([](auto &item)
  {
    return item.get_id() == 49517;
  });

  auto non_soil_count = inventory.count([](auto &item)
  {
    return item.get_id() != 49517;
  });

  auto total_count = inventory.count();
  auto effectively_full = (non_soil_count >= 22 || (total_count >= 22 && Api::get_world_setting(9370) == 100));
  if (effectively_full)
  {
    auto is_at_bank = self.tile_position() == Vec2<uint32_t>(3356, 3197);
    if (!is_at_bank && soil_box.has_value() && has_soil && Api::get_world_setting(9370) < 100)
    {
      Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::widget, 0),
          soil_box->create_menu_action_args(1));
    }
    else
    {
      if (soil_box.has_value() && Api::get_world_setting(9370) != 0)
      {
        if (!Api::has_selected_item())
        {
          Api::select_item(1473, 5, soil_box->get_slot());
        }

        Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::obj, 6),
            { 115427, 3356, 3198, 1 });
      }
      else
      {
        Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::obj, 3),
            { 115427, 3356, 3198, 1 });
      }
    }
  }
  else
  {
    if (soil_box.has_value() && has_soil && Api::get_world_setting(9370) < 100)
    {
      Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::widget, 0),
          soil_box->create_menu_action_args(1));
    }
    else
    {
      Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::obj, 0),
          { 117100, 3370, 3189, 1 });
    }
  }
}

void archaeology_override(MenuActionEventArgs *args, int32_t site, int32_t spot)
{
  if (site == arch_site_kharid_et)
  {
    if (spot == kharid_et_main_entrance)
    {
      return khard_et_main_entrance_override(args);
    }
  }
}

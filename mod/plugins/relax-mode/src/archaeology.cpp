#include "archaeology.h"

namespace
{
  constexpr int32_t soil_box_item = 49538;
  constexpr int32_t soil_item = 49517;
  constexpr uint32_t soil_box_fill = 9370;
  constexpr uint32_t material_storage = 884;

  constexpr uint16_t bank_soil_parent = 517;
  constexpr uint16_t bank_soil_child = 15;
  constexpr uint16_t material_storage_parent = 1370;
  constexpr uint16_t material_storage_child = 30;

  constexpr uint32_t sift_object = 115427;
  constexpr uint32_t sift_x = 3363;
  constexpr uint32_t sift_y = 3397;

  constexpr uint32_t material_cart = 115420;
  constexpr uint32_t material_cart_x = 3378;
  constexpr uint32_t material_cart_y = 3384;

  constexpr uint32_t kharid_bank = 115427;
  constexpr uint32_t kharid_bank_x = 3356;
  constexpr uint32_t kharid_bank_y = 3198;

  constexpr uint32_t kharid_dig = 117100;
  constexpr uint32_t kharid_dig_x = 3370;
  constexpr uint32_t kharid_dig_y = 3189;

  std::optional<ApiItem> find_soil_box(const ApiItemContainer &inventory)
  {
    return inventory.find(soil_box_item);
  }
} // namespace

static void sift_override()
{
  auto inventory = Api::get_inventory();
  if (!inventory.has_value())
  {
    return;
  }

  auto soil_box = find_soil_box(*inventory);

  if (Api::get_world_setting(soil_box_fill) == 0 || inventory->is_full())
  {
    if (inventory->count() != 1)
    {
      Api::override_object(sift_object, sift_x, sift_y, 3);
    }
    else if (Api::get_bank().has_value() && soil_box.has_value())
    {
      Api::override_widget(bank_soil_parent, bank_soil_child, soil_box->get_slot(), InventoryOption::drop);
    }
    else
    {
      Api::override_object(sift_object, sift_x, sift_y, 1);
    }
  }
  else if (Api::get_item_container(material_storage).has_value())
  {
    Api::override_widget(material_storage_parent, material_storage_child, -1, 0, 2);
  }
  else
  {
    Api::override_object(material_cart, material_cart_x, material_cart_y);
  }
}

static void khard_et_main_entrance_override()
{
  auto self = Api::self();
  auto inventory = Api::get_inventory();
  if (!self.has_value() || !inventory.has_value())
  {
    return;
  }

  auto soil_box = find_soil_box(*inventory);
  auto has_soil = inventory->contains(soil_item);
  auto non_soil_count = inventory->count([](auto &item)
  {
    return item.get_id() != soil_item;
  });

  auto total_count = inventory->count();
  auto fill = Api::get_world_setting(soil_box_fill);
  auto effectively_full = (non_soil_count >= 22 || (total_count >= 22 && fill == 100));
  if (effectively_full)
  {
    auto is_at_bank = self->tile_position() == Vec2<uint32_t>(3356, 3197);
    if (!is_at_bank && soil_box.has_value() && has_soil && fill < 100)
    {
      soil_box->override_interact(1);
    }
    else if (soil_box.has_value() && fill != 0)
    {
      if (!Api::has_selected_item())
      {
        soil_box->select();
      }

      Api::override_object(kharid_bank, kharid_bank_x, kharid_bank_y, 6);
    }
    else
    {
      Api::override_object(kharid_bank, kharid_bank_x, kharid_bank_y, 3);
    }
  }
  else if (soil_box.has_value() && has_soil && fill < 100)
  {
    soil_box->override_interact(1);
  }
  else
  {
    Api::override_object(kharid_dig, kharid_dig_x, kharid_dig_y);
  }
}

void archaeology_override(int32_t site, int32_t spot)
{
  if (site == arch_site_sift)
  {
    return sift_override();
  }
  else if (site == arch_site_kharid_et)
  {
    if (spot == kharid_et_main_entrance)
    {
      return khard_et_main_entrance_override();
    }
  }
}

#include "plugin_cpp.h"

namespace crs
{
  ApiItem::ApiItem(uint16_t parent_widget, uint16_t child_widget, int32_t slot, int32_t id, int32_t amount) : parent_widget(parent_widget),
                                                                                                              child_widget(child_widget),
                                                                                                              slot(slot),
                                                                                                              id(id),
                                                                                                              amount(amount)
  {
  }

  MenuActionArgs ApiItem::create_menu_action_args(int index) const
  {
    MenuActionArgs args;
    args.args_widget.option_idx = index;
    args.args_widget.sub_idx = slot;
    args.args_widget.widget_id = (static_cast<uint32_t>(parent_widget) << 16) | child_widget;
    args.args_widget.always_1 = 1;
    return args;
  }

  void ApiItem::interact(uint32_t option, uint32_t handler) const
  {
    if (empty())
    {
      return;
    }

    Api::perform_menu_action(Api::get_menu_action_handler(MenuActionType::widget, handler), create_menu_action_args(static_cast<int>(option)));
  }

  void ApiItem::override_interact(uint32_t option, uint32_t handler) const
  {
    if (empty())
    {
      return;
    }

    Api::override_current_menu_action(Api::get_menu_action_handler(MenuActionType::widget, handler), create_menu_action_args(static_cast<int>(option)));
  }

  void ApiItem::select() const
  {
    if (empty())
    {
      return;
    }

    Api::select_item(parent_widget, child_widget, slot);
  }

  bool ApiItem::empty() const
  {
    return id == -1;
  }

  int32_t ApiItem::get_id() const
  {
    return id;
  }

  int32_t ApiItem::get_amount() const
  {
    return amount;
  }

  int32_t ApiItem::get_slot() const
  {
    return slot;
  }

  ApiItemContainer::ApiItemContainer() : capacity(0)
  {
  }

  ApiItemContainer::ApiItemContainer(uint32_t container_id, uint32_t capacity, const std::vector<ApiItem> &items) : container_id(container_id),
                                                                                                                    capacity(capacity),
                                                                                                                    items(items)
  {
  }

  ApiItemContainer::ApiItemContainer(uint32_t capacity, const std::vector<ApiItem> &items) : ApiItemContainer(0, capacity, items)
  {
  }

  uint32_t ApiItemContainer::id() const
  {
    return container_id;
  }

  const std::vector<ApiItem> &ApiItemContainer::all() const
  {
    return items;
  }

  std::optional<ApiItem> ApiItemContainer::first(std::function<bool(const ApiItem &)> conditional) const
  {
    std::optional<ApiItem> op;
    for (auto &item : items)
    {
      if (conditional(item))
      {
        op = item;
        break;
      }
    }
    return op;
  }

  std::optional<ApiItem> ApiItemContainer::find(int32_t item_id) const
  {
    return first([item_id](const ApiItem &item)
    {
      return item.get_id() == item_id;
    });
  }

  size_t ApiItemContainer::count(std::function<bool(const ApiItem &)> conditional) const
  {
    auto count = 0ull;
    for (auto &item : items)
    {
      if (conditional(item))
      {
        count += 1;
      }
    }
    return count;
  }

  bool ApiItemContainer::contains(std::function<bool(const ApiItem &)> conditional) const
  {
    for (auto &item : items)
    {
      if (conditional(item))
      {
        return true;
      }
    }
    return false;
  }

  bool ApiItemContainer::contains(int32_t item_id) const
  {
    return find(item_id).has_value();
  }

  bool ApiItemContainer::is_full() const
  {
    return items.size() == capacity;
  }

  std::vector<ApiItem> ApiItemContainer::filter(std::function<bool(const ApiItem &)> condition) const
  {
    std::vector<ApiItem> f;
    for (auto item : items)
    {
      if (condition(item))
      {
        f.push_back(item);
      }
    }
    return f;
  }
} // namespace crs

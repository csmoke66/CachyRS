#include "plugin_cpp.h"

namespace crs
{
  ApiItem::ApiItem(uint32_t encoded_widget, int32_t slot, int32_t id, int32_t amount)
  {
    this->encoded_widget = encoded_widget;
    this->slot = slot;
    this->id = id;
    this->amount = amount;
  }

  ApiItem::ApiItem(const ApiItem &o) : ApiItem(o.encoded_widget, o.slot, o.id, o.amount)
  {
  }

  MenuActionArgs ApiItem::fill_menu_action_args(int index)
  {
    MenuActionArgs args;
    args.args_widget.option_idx = index;
    args.args_widget.sub_idx = slot;
    args.args_widget.widget_id = encoded_widget;
    args.args_widget.always_1 = 1;
    return args;
  }

  int32_t ApiItem::get_id()
  {
    return this->id;
  }

  int32_t ApiItem::get_amount()
  {
    return this->amount;
  }

  ApiItemContainer::ApiItemContainer()
  {
    this->capacity = 0;
  }

  ApiItemContainer::ApiItemContainer(uint32_t capacity, const std::vector<ApiItem> &items)
  {
    this->capacity = capacity;
    this->items = items;
  }

  ApiItemContainer::ApiItemContainer(const ApiItemContainer &o) : ApiItemContainer(o.capacity, o.items)
  {
  }

  std::optional<ApiItem> ApiItemContainer::first(std::function<bool(ApiItem &)> conditional)
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

  size_t ApiItemContainer::count(std::function<bool(ApiItem &)> conditional)
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

  bool ApiItemContainer::contains(std::function<bool(ApiItem &)> conditional)
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

  bool ApiItemContainer::is_full()
  {
    return items.size() == capacity;
  }
} // namespace crs
#include "plugin_cpp.h"

namespace crs
{
  ApiComponent::ApiComponent(PluginApi api, uint64_t id)
  {
    this->api = api;
    this->id = id;
  }

  ApiComponent::ApiComponent(const ApiComponent &o)
  {
  }

  ApiContainer::ApiContainer() : ApiComponent({ nullptr }, static_cast<uint64_t>(-1))
  {
  }

  ApiContainer::ApiContainer(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiContainer::ApiContainer(const ApiContainer &o) : ApiContainer(o.api, o.id)
  {
  }

  ApiContainer ApiContainer::add_container()
  {
    return Api::add_container(this->id);
  }

  ApiLabel ApiContainer::add_label(const std::string &text)
  {
    return Api::add_label(this->id, text);
  }

  ApiHr ApiContainer::add_hr()
  {
    return Api::add_hr(this->id);
  }

  ApiCheckBox ApiContainer::add_checkbox(const std::string &text)
  {
    return Api::add_checkbox(this->id, text);
  }

  std::shared_ptr<ApiDropDown> ApiContainer::add_dropdown(std::vector<std::string> options)
  {
    return Api::add_dropdown(this->id, options);
  }

  ApiLabel::ApiLabel(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiLabel::ApiLabel(const ApiLabel &o) : ApiLabel(o.api, o.id)
  {
  }

  ApiHr::ApiHr(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
    this->api = api;
    this->id = id;
  }

  ApiHr::ApiHr(const ApiHr &o) : ApiHr(o.api, o.id)
  {
  }

  ApiCheckBox::ApiCheckBox() : ApiComponent({ nullptr }, static_cast<uint64_t>(-1))
  {
  }

  ApiCheckBox::ApiCheckBox(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiCheckBox::ApiCheckBox(const ApiCheckBox &o) : ApiCheckBox(o.api, o.id)
  {
  }

  bool ApiCheckBox::is_checked()
  {
    return api.ui_is_component_checked(id);
  }

  ApiDropDown::ApiDropDown()
  {
    this->id = static_cast<uint64_t>(-1);
  }

  ApiDropDown::ApiDropDown(PluginApi api, uint64_t id)
  {
    this->api = api;
    this->id = id;
  }

  ApiDropDown::ApiDropDown(const ApiDropDown &o) : ApiDropDown(o.api, o.id)
  {
    this->change_handlers = o.change_handlers;
  }

  void ApiDropDown::fire_changed(int32_t idx)
  {
    this->selected = idx;
    for (auto &handler : this->change_handlers)
    {
      handler(idx);
    }
  }

  void ApiDropDown::on_changed(std::function<void(int32_t)> f)
  {
    this->change_handlers.push_back(f);
  }

  int ApiDropDown::get_selected()
  {
    return selected;
  }

  bool ApiDropDown::is_selected(int32_t index)
  {
    return selected == index;
  }
} // namespace crs
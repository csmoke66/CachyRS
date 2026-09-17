#include "plugin_cpp.h"

namespace crs
{
  ApiComponent::ApiComponent(PluginApi api, uint64_t id) : api(api),
                                                           id(id)
  {
  }

  ApiContainer::ApiContainer() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiContainer::ApiContainer(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiContainer ApiContainer::add_container()
  {
    return Api::add_container(id);
  }

  ApiLabel ApiContainer::add_label(const std::string &text)
  {
    return Api::add_label(id, text);
  }

  ApiHr ApiContainer::add_hr()
  {
    return Api::add_hr(id);
  }

  ApiCheckBox ApiContainer::add_checkbox(const std::string &text)
  {
    return Api::add_checkbox(id, text);
  }

  std::shared_ptr<ApiDropDown> ApiContainer::add_dropdown(std::vector<std::string> options)
  {
    return Api::add_dropdown(id, options);
  }

  ApiLabel::ApiLabel() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiLabel::ApiLabel(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  void ApiLabel::set_text(const std::string &text)
  {
    api.ui_update_component_text(id, text.c_str());
  }

  ApiHr::ApiHr(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiCheckBox::ApiCheckBox() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiCheckBox::ApiCheckBox(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  bool ApiCheckBox::is_checked() const
  {
    return api.ui_is_component_active(id);
  }

  void ApiCheckBox::set_checked(bool checked)
  {
    api.ui_set_component_active(id, checked);
  }

  ApiDropDown::ApiDropDown() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiDropDown::ApiDropDown(PluginApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  void ApiDropDown::fire_changed(int32_t idx)
  {
    selected = idx;
    for (auto &handler : change_handlers)
    {
      handler(idx);
    }
  }

  void ApiDropDown::on_changed(std::function<void(int32_t)> handler)
  {
    change_handlers.push_back(std::move(handler));
  }

  int32_t ApiDropDown::get_selected() const
  {
    return selected;
  }

  bool ApiDropDown::is_selected(int32_t index) const
  {
    return selected == index;
  }

  void ApiDropDown::set_selected(int32_t index)
  {
    selected = index;
    api.ui_dropdown_set_selected(id, index);
  }

  int32_t DropDownContentChanger::get_selected() const
  {
    if (!dropdown)
    {
      return 0;
    }

    return dropdown->get_selected();
  }

  bool DropDownContentChanger::is_selected(int32_t idx) const
  {
    return dropdown && dropdown->is_selected(idx);
  }

  void DropDownContentChanger::on_changed(int32_t idx)
  {
    if (idx < 0 || static_cast<size_t>(idx) >= containers.size())
    {
      return;
    }

    if (last_visible)
    {
      last_visible->set_visible(false);
      last_visible = nullptr;
    }

    last_visible = containers[static_cast<size_t>(idx)];
    if (last_visible)
    {
      last_visible->set_visible(true);
    }
  }

  void DropDownContentChanger::reset()
  {
    for (auto container : containers)
    {
      if (container)
      {
        container->set_visible(false);
      }
    }

    last_visible = nullptr;
    if (!containers.empty() && containers[0])
    {
      containers[0]->set_visible(true);
      last_visible = containers[0];
    }
  }

  std::unique_ptr<DropDownContentChanger> DropDownContentChanger::new_changer(
      uint64_t parent,
      std::vector<DropDownContentOption> options)
  {
    auto pages = std::make_unique<DropDownContentChanger>();
    std::vector<std::string> names;
    names.reserve(options.size());
    pages->containers.reserve(options.size());
    for (auto &option : options)
    {
      names.push_back(option.name);
      pages->containers.push_back(option.container);
    }

    auto raw = pages.get();
    pages->dropdown = Api::add_dropdown(parent, names);
    pages->dropdown->on_changed([raw](int32_t idx)
    {
      raw->on_changed(idx);
    });

    return pages;
  }
} // namespace crs
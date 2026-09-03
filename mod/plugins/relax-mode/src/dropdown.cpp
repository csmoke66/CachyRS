#include "relax_mode.h"

DropDownContentChanger::DropDownContentChanger()
{
}

int32_t DropDownContentChanger::get_selected()
{
  return dropdown->get_selected();
}

bool DropDownContentChanger::is_selected(int32_t idx)
{
  return dropdown->is_selected(idx);
}

void DropDownContentChanger::on_changed(int32_t idx)
{
  if (last_visible)
  {
    last_visible->set_visible(false);
    last_visible = nullptr;
  }

  last_visible = containers[idx];
  if (last_visible)
  {
    last_visible->set_visible(true);
  }
}

void DropDownContentChanger::reset()
{
  for (auto container : containers)
  {
    container->set_visible(false);
  }

  containers[0]->set_visible(true);
}

std::unique_ptr<DropDownContentChanger> DropDownContentChanger::new_changer(
    uint64_t parent,
    std::vector<DropDownContentOption> options)
{
  auto n = std::make_unique<DropDownContentChanger>();
  std::vector<std::string> names;
  for (auto &o : options)
  {
    names.push_back(o.name);
    n->containers.push_back(o.container);
  }

  auto raw = n.get();
  n->dropdown = Api::add_dropdown(parent, names);
  n->dropdown->on_changed([raw](auto idx)
  {
    raw->on_changed(idx);
  });

  return std::move(n);
}
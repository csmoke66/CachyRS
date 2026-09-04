#pragma once
#include "event_bus.h"
#include "plugin.h"
#include <format>
#include <plugin_cpp.h>

using namespace crs;

struct DropDownContentOption
{
  std::string name;
  ApiContainer *container;
};

class DropDownContentChanger
{
private:
  std::shared_ptr<ApiDropDown> dropdown;
  ApiContainer *last_visible = nullptr;
  std::vector<ApiContainer *> containers;

public:
  DropDownContentChanger();

public:
  int32_t get_selected();
  bool is_selected(int32_t idx);

public:
  void on_changed(int32_t idx);
  void reset();

public:
  static std::unique_ptr<DropDownContentChanger> new_changer(
      uint64_t parent,
      std::vector<DropDownContentOption> options);
};

enum RelaxMode : int32_t
{
  relax_mode_test,
  relax_mode_archaeology
};

enum ArchaeologySite : int32_t
{
  arch_site_kharid_et
};

enum KhardEtSpot : int32_t
{
  kharid_et_main_entrance
};

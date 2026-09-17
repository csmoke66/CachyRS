#include "ui.h"

namespace crs
{
  UserInterface::~UserInterface() = default;

  void UserInterface::reload()
  {
  }

  void UserInterface::add_reload_callback(std::function<void()>)
  {
  }

  void UserInterface::request_verify()
  {
    wants_verify = true;
  }

  bool UserInterface::check_verify()
  {
    if (wants_verify)
    {
      wants_verify = false;
      return true;
    }

    return false;
  }
} // namespace crs
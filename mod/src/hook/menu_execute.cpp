#include "cachy.h"

namespace crs
{
  std::string MenuExecuteHook::get_action_handler_name(FnMenuActionHandler handler)
  {
    auto offset = reinterpret_cast<uint64_t>(handler) - reinterpret_cast<uint64_t>(RS.get_globals().unwrap());
    switch (offset)
    {
    case off(Globals, menu_action_handler_widget0):
      return "menu_action_handler_widget0";
    case off(Globals, menu_action_handler_widget1):
      return "menu_action_handler_widget1";
    case off(Globals, menu_action_handler_widget2):
      return "menu_action_handler_widget2";
    case off(Globals, menu_action_handler_walk):
      return "menu_action_handler_walk";
    case off(Globals, menu_action_handler_npc6):
      return "menu_action_handler_npc6";
    case off(Globals, menu_action_handler_npc5):
      return "menu_action_handler_npc5";
    case off(Globals, menu_action_handler_npc4):
      return "menu_action_handler_npc4";
    case off(Globals, menu_action_handler_npc3):
      return "menu_action_handler_npc3";
    case off(Globals, menu_action_handler_npc2):
      return "menu_action_handler_npc2";
    case off(Globals, menu_action_handler_npc1):
      return "menu_action_handler_npc1";
    case off(Globals, menu_action_handler_npc0):
      return "menu_action_handler_obj0";
    case off(Globals, menu_action_handler_obj6):
      return "menu_action_handler_obj6";
    case off(Globals, menu_action_handler_obj5):
      return "menu_action_handler_obj5";
    case off(Globals, menu_action_handler_obj4):
      return "menu_action_handler_obj4";
    case off(Globals, menu_action_handler_obj3):
      return "menu_action_handler_obj3";
    case off(Globals, menu_action_handler_obj2):
      return "menu_action_handler_obj2";
    case off(Globals, menu_action_handler_obj1):
      return "menu_action_handler_obj1";
    case off(Globals, menu_action_handler_obj0):
      return "menu_action_handler_obj0";
    }
    return "{unknown}";
  }

  std::string MenuExecuteHook::get_action_handler_api(FnMenuActionHandler handler)
  {
    auto offset = reinterpret_cast<uint64_t>(handler) - reinterpret_cast<uint64_t>(RS.get_globals().unwrap());
    switch (offset)
    {
    case off(Globals, menu_action_handler_widget0):
      return "Api::get_menu_action_handler(MenuActionType::widget, 0)";
    case off(Globals, menu_action_handler_widget1):
      return "Api::get_menu_action_handler(MenuActionType::widget, 1)";
    case off(Globals, menu_action_handler_widget2):
      return "Api::get_menu_action_handler(MenuActionType::widget, 2)";
    case off(Globals, menu_action_handler_walk):
      return "Api::get_menu_action_handler(MenuActionType::walk)";
    case off(Globals, menu_action_handler_npc6):
      return "Api::get_menu_action_handler(MenuActionType::npc, 6)";
    case off(Globals, menu_action_handler_npc5):
      return "Api::get_menu_action_handler(MenuActionType::npc, 5)";
    case off(Globals, menu_action_handler_npc4):
      return "Api::get_menu_action_handler(MenuActionType::npc, 4)";
    case off(Globals, menu_action_handler_npc3):
      return "Api::get_menu_action_handler(MenuActionType::npc, 3)";
    case off(Globals, menu_action_handler_npc2):
      return "Api::get_menu_action_handler(MenuActionType::npc, 2)";
    case off(Globals, menu_action_handler_npc1):
      return "Api::get_menu_action_handler(MenuActionType::npc, 1)";
    case off(Globals, menu_action_handler_npc0):
      return "Api::get_menu_action_handler(MenuActionType::npc, 0)";
    case off(Globals, menu_action_handler_obj6):
      return "Api::get_menu_action_handler(MenuActionType::obj, 6)";
    case off(Globals, menu_action_handler_obj5):
      return "Api::get_menu_action_handler(MenuActionType::obj, 5)";
    case off(Globals, menu_action_handler_obj4):
      return "Api::get_menu_action_handler(MenuActionType::obj, 4)";
    case off(Globals, menu_action_handler_obj3):
      return "Api::get_menu_action_handler(MenuActionType::obj, 3)";
    case off(Globals, menu_action_handler_obj2):
      return "Api::get_menu_action_handler(MenuActionType::obj, 2)";
    case off(Globals, menu_action_handler_obj1):
      return "Api::get_menu_action_handler(MenuActionType::obj, 1)";
    case off(Globals, menu_action_handler_obj0):
      return "Api::get_menu_action_handler(MenuActionType::obj, 0)";
    }
    return "{unknown}";
  }
  
  void MenuExecuteHook::print_action_handler(MenuActionContext *menu_action_context)
  {
    auto tmpl = menu_action_context->tmpl;
    auto args = menu_action_context->args.r;

    LOG(INFO, " -> menu execute");
    LOG(INFO, "  -> off: " << std::hex << RS.pi.offset((void *)tmpl->handler)
                               << std::dec
                               << " type: " << static_cast<uint32_t>(tmpl->type)
                               << " id:" << tmpl->id);

    LOG(INFO, std::format("Api::perform_menu_action({}, {{ {}, {}, {}, {} }})", 
      get_action_handler_api(tmpl->handler),
      args[0], args[1], args[2], args[3]
    ));
  }

  void MenuExecuteHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto menu_action = (MenuAction *)CPU_SECOND_ARG(cpu_state);
    auto menu_action_context = menu_action->menu_action_context;

    auto pre_event = MenuActionEvent(MenuActionEvent::pre_id(), &menu_action_context->args, &menu_action_context->tmpl);
    RS.event_bus.dispatch(MenuActionEvent::pre_id(), &pre_event);

    print_action_handler(menu_action_context);

    if (pre_event.args.bypass_logic)
    {
      cpu_state->rax = reinterpret_cast<uint64_t>(menu_action_context->tmpl->handler(menu_action_context->tmpl, menu_action));
    }
    else
    {
      cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
          reinterpret_cast<void *>(CPU_FIRST_ARG(cpu_state)),
          menu_action,
          reinterpret_cast<void *>(CPU_THIRD_ARG(cpu_state))));
    }

    auto post_event = MenuActionEvent(MenuActionEvent::post_id(), &menu_action_context->args, &menu_action_context->tmpl);
    RS.event_bus.dispatch(MenuActionEvent::post_id(), &post_event);
  }
} // namespace crs
#include "plugin_cpp.h"

#include <iterator>

namespace crs
{
  namespace
  {
    constexpr uint32_t walk_arg = 0x6d200001;

    constexpr std::string_view k_skill_names[] = {
      "Attack",
      "Defence",
      "Strength",
      "Constitution",
      "Ranged",
      "Prayer",
      "Magic",
      "Cooking",
      "Woodcutting",
      "Fletching",
      "Fishing",
      "Firemaking",
      "Crafting",
      "Smithing",
      "Mining",
      "Herblore",
      "Agility",
      "Thieving",
      "Slayer",
      "Farming",
      "Runecrafting",
      "Hunter",
      "Construction",
      "Summoning",
      "Dungeoneering",
      "Divination",
      "Invention",
      "Archaeology",
      "Necromancy",
    };

    template <typename Fn>
    void for_each_widget(WidgetCache *cache, Fn &&fn)
    {
      if (!cache)
      {
        return;
      }

      for (auto widget_001 = cache->c.begin; widget_001 != cache->c.end; widget_001++)
      {
        if (auto widget_002 = widget_001->widget_002)
        {
          for (auto widget_003 = widget_002->widgets_003.begin; widget_003 != widget_002->widgets_003.end; widget_003++)
          {
            if (auto widget = widget_003->widget)
            {
              fn(widget);
            }
          }
        }
      }
    }

    void collect_entities(WorldNode *node, std::optional<EntityType> type, std::vector<ApiEntity> &out)
    {
      if (!node)
      {
        return;
      }

      if (auto entity = node->entity)
      {
        if (!type.has_value() || entity->type == *type)
        {
          out.push_back(ApiEntity(entity));
        }
      }

      for (auto child = node->children.begin; child != node->children.end; child++)
      {
        collect_entities(*child, type, out);
      }
    }

    MenuActionArgs walk_args(uint32_t tile_x, uint32_t tile_y)
    {
      MenuActionArgs args{};
      args.r[0] = 0;
      args.r[1] = static_cast<int32_t>(tile_x);
      args.r[2] = static_cast<int32_t>(tile_y);
      args.r[3] = static_cast<int32_t>(walk_arg);
      return args;
    }

    MenuActionArgs object_args(uint32_t object_id, uint32_t tile_x, uint32_t tile_y)
    {
      MenuActionArgs args{};
      args.args_obj.object_id = object_id;
      args.args_obj.tile_x = tile_x;
      args.args_obj.tile_y = tile_y;
      args.args_obj.always_1 = 1;
      return args;
    }

    MenuActionArgs npc_args(uint32_t server_index)
    {
      MenuActionArgs args{};
      args.args_npc.server_idx = server_index;
      args.args_npc.always_0_0 = 0;
      args.args_npc.always_0_1 = 0;
      args.args_npc.always_1 = 1;
      return args;
    }

    MenuActionArgs widget_args(uint16_t parent, uint16_t child, int32_t slot, uint32_t option)
    {
      MenuActionArgs args{};
      args.args_widget.option_idx = option;
      args.args_widget.sub_idx = static_cast<uint32_t>(slot);
      args.args_widget.widget_id = (static_cast<uint32_t>(parent) << 16) | child;
      args.args_widget.always_1 = 1;
      return args;
    }
  } // namespace

  Scene003 *Api::raw_scene()
  {
    auto engine = raw_engine();
    if (!engine)
    {
      return nullptr;
    }

    auto scene_001 = engine->scene_001;
    if (!scene_001)
    {
      return nullptr;
    }

    auto scene_002 = scene_001->scene_002.reference(static_cast<size_t>(scene_001->scene_index));
    if (!scene_002)
    {
      return nullptr;
    }

    return scene_002->scene_003;
  }

  WorldNode *Api::raw_world_root()
  {
    if (auto scene = raw_scene())
    {
      return scene->world_root;
    }

    return nullptr;
  }

  Menu *Api::raw_menu()
  {
    auto engine = raw_engine();
    if (!engine)
    {
      return nullptr;
    }

    return engine->menu;
  }

  SDL_Window *Api::raw_sdl_window()
  {
    return dref<SDL_Window *>(
        raw_globals(),
        { off(Globals, linux_001),
            off(Linux001, linux_002),
            off(Linux002, linux_003),
            off(Linux003, linux_004),
            off(Linux004, linux_005),
            off(Linux005, sdl_window) });
  }

  GameState Api::game_state()
  {
    if (auto engine = raw_engine())
    {
      return engine->state;
    }

    return GameState::login_screen;
  }

  bool Api::in_game()
  {
    return game_state() == GameState::in_game;
  }

  uint32_t Api::engine_time()
  {
    if (auto engine = raw_engine())
    {
      return engine->time;
    }

    return 0;
  }

  std::string Api::local_name()
  {
    auto engine = raw_engine();
    if (!engine)
    {
      return {};
    }

    auto lp = engine->local_player;
    if (!lp)
    {
      return {};
    }

    return lp->name;
  }

  uint32_t Api::run_energy()
  {
    if (auto lpv = raw_local_player_variables())
    {
      return lpv->run_energy;
    }

    return 0;
  }

  std::span<const std::string_view> Api::skill_names()
  {
    return k_skill_names;
  }

  std::string_view Api::skill_name(uint32_t index)
  {
    if (index >= std::size(k_skill_names))
    {
      return {};
    }

    return k_skill_names[index];
  }

  std::vector<Skill> Api::skills()
  {
    std::vector<Skill> out;
    auto lpv = raw_local_player_variables();
    if (!lpv || !lpv->stats)
    {
      return out;
    }

    out.reserve(lpv->stat_count);
    for (uint32_t i = 0; i < lpv->stat_count; i++)
    {
      auto stat = lpv->stats[i];
      Skill skill;
      skill.index = i;
      skill.name = skill_name(i);
      skill.current = stat.current_level;
      skill.max = stat.max_level;
      skill.xp = stat.experience;
      out.push_back(skill);
    }

    return out;
  }

  std::optional<Skill> Api::skill(uint32_t index)
  {
    auto lpv = raw_local_player_variables();
    if (!lpv || !lpv->stats || index >= lpv->stat_count)
    {
      return std::nullopt;
    }

    auto stat = lpv->stats[index];
    Skill skill;
    skill.index = index;
    skill.name = skill_name(index);
    skill.current = stat.current_level;
    skill.max = stat.max_level;
    skill.xp = stat.experience;
    return skill;
  }

  std::vector<ApiEntity> Api::scene_entities(std::optional<EntityType> type)
  {
    std::vector<ApiEntity> out;
    collect_entities(raw_world_root(), type, out);
    return out;
  }

  std::optional<ApiItemContainer> Api::get_bank()
  {
    return get_item_container(Containers::bank);
  }

  std::vector<FriendView> Api::friends()
  {
    std::vector<FriendView> out;
    auto cache = raw_social_cache();
    if (!cache)
    {
      return out;
    }

    for (auto friend_ = cache->friends.begin; friend_ != cache->friends.end; friend_++)
    {
      FriendView view;
      view.name = friend_->name.str();
      view.previous_name = friend_->previous_name.str();
      view.world = friend_->world.str();
      out.push_back(view);
    }

    return out;
  }

  std::vector<std::string> Api::ignored()
  {
    std::vector<std::string> out;
    auto cache = raw_social_cache();
    if (!cache)
    {
      return out;
    }

    for (auto entry = cache->ignored.begin; entry != cache->ignored.end; entry++)
    {
      out.push_back(entry->name.str());
    }

    return out;
  }

  bool Api::is_friend(const std::string &name)
  {
    for (auto &friend_ : friends())
    {
      if (friend_.name == name)
      {
        return true;
      }
    }

    return false;
  }

  bool Api::menu_open()
  {
    if (auto menu = raw_menu())
    {
      return menu->is_open != 0;
    }

    return false;
  }

  ApiWidget::ApiWidget(Widget *widget) : widget(widget)
  {
  }

  bool ApiWidget::valid() const
  {
    return widget != nullptr;
  }

  ApiWidget::operator bool() const
  {
    return valid();
  }

  Widget *ApiWidget::raw() const
  {
    return widget;
  }

  uint16_t ApiWidget::parent_id() const
  {
    if (!widget)
    {
      return 0;
    }

    return widget->parent_id;
  }

  uint16_t ApiWidget::child_id() const
  {
    if (!widget)
    {
      return 0;
    }

    return widget->child_id;
  }

  WidgetType ApiWidget::type() const
  {
    if (!widget)
    {
      return WidgetType::container;
    }

    return widget->get_type();
  }

  uint32_t ApiWidget::x() const
  {
    if (!widget)
    {
      return 0;
    }

    return widget->x;
  }

  uint32_t ApiWidget::y() const
  {
    if (!widget)
    {
      return 0;
    }

    return widget->y;
  }

  uint32_t ApiWidget::width() const
  {
    if (!widget)
    {
      return 0;
    }

    return widget->width;
  }

  uint32_t ApiWidget::height() const
  {
    if (!widget)
    {
      return 0;
    }

    return widget->height;
  }

  std::optional<ApiWidget> ApiWidget::parent() const
  {
    if (!widget || !widget->parent)
    {
      return std::nullopt;
    }

    return ApiWidget(widget->parent);
  }

  std::vector<ApiWidget> ApiWidget::children() const
  {
    std::vector<ApiWidget> out;
    if (!widget)
    {
      return out;
    }

    for_each_widget(Api::raw_widget_cache(), [this, &out](Widget *child)
    {
      if (static_cast<Widget *>(child->parent) == widget)
      {
        out.push_back(ApiWidget(child));
      }
    });

    return out;
  }

  std::optional<ApiWidget> Api::widget(uint16_t parent, uint16_t child)
  {
    std::optional<ApiWidget> found;
    for_each_widget(raw_widget_cache(), [&](Widget *w)
    {
      if (!found && w->parent_id == parent && w->child_id == child)
      {
        found = ApiWidget(w);
      }
    });

    return found;
  }

  std::optional<Vec2<int32_t>> Api::window_size()
  {
    auto window = raw_sdl_window();
    if (!window)
    {
      return std::nullopt;
    }

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0)
    {
      return std::nullopt;
    }

    return Vec2<int32_t>{ width, height };
  }

  std::optional<Vec2<float>> Api::world_to_screen(const Vec3<float> &scene)
  {
    auto scene_003 = raw_scene();
    auto bounds = window_size();
    if (!scene_003 || !bounds)
    {
      return std::nullopt;
    }

    auto matrix = scene_003->projection_matrix;
    auto w = scene.x * matrix.flat[3] + scene.y * matrix.flat[7] + scene.z * matrix.flat[11] + matrix.flat[15];
    if (w < 0.1f)
    {
      return std::nullopt;
    }

    Vec3<float> clip;
    clip.x = scene.x * matrix.flat[0] + scene.y * matrix.flat[4] + scene.z * matrix.flat[8] + matrix.flat[12];
    clip.y = scene.x * matrix.flat[1] + scene.y * matrix.flat[5] + scene.z * matrix.flat[9] + matrix.flat[13];

    auto ndc_x = clip.x / w;
    auto ndc_y = clip.y / w;
    auto width = static_cast<float>(bounds->x);
    auto height = static_cast<float>(bounds->y);

    return Vec2<float>{
      (width / 2.f * ndc_x) + (ndc_x + width / 2.f),
      -(height / 2.f * ndc_y) + (ndc_y + height / 2.f)
    };
  }

  void Api::walk(uint32_t tile_x, uint32_t tile_y)
  {
    perform_menu_action(get_menu_action_handler(MenuActionType::walk), walk_args(tile_x, tile_y));
  }

  void Api::interact_object(uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option)
  {
    perform_menu_action(get_menu_action_handler(MenuActionType::obj, option), object_args(object_id, tile_x, tile_y));
  }

  void Api::interact_npc(uint32_t server_index, uint32_t option)
  {
    perform_menu_action(get_menu_action_handler(MenuActionType::npc, option), npc_args(server_index));
  }

  void Api::interact_widget(uint16_t parent, uint16_t child, int32_t slot, uint32_t option, uint32_t handler)
  {
    perform_menu_action(get_menu_action_handler(MenuActionType::widget, handler), widget_args(parent, child, slot, option));
  }

  void Api::override_walk(uint32_t tile_x, uint32_t tile_y)
  {
    override_current_menu_action(get_menu_action_handler(MenuActionType::walk), walk_args(tile_x, tile_y));
  }

  void Api::override_object(uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option)
  {
    override_current_menu_action(get_menu_action_handler(MenuActionType::obj, option), object_args(object_id, tile_x, tile_y));
  }

  void Api::override_npc(uint32_t server_index, uint32_t option)
  {
    override_current_menu_action(get_menu_action_handler(MenuActionType::npc, option), npc_args(server_index));
  }

  void Api::override_widget(uint16_t parent, uint16_t child, int32_t slot, uint32_t option, uint32_t handler)
  {
    override_current_menu_action(get_menu_action_handler(MenuActionType::widget, handler), widget_args(parent, child, slot, option));
  }
} // namespace crs

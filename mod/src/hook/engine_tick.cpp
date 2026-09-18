#include "cachy.h"
#include "not_cachy.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

namespace crs
{
  namespace
  {
    template <typename T>
    [[clang::optnone]]
    void touch(T value)
    {
      volatile T sink = value;
      (void)sink;
    }

    [[clang::optnone]]
    void touch_cstr(const char *s)
    {
      if (s)
      {
        touch(s[0]);
      }
    }

    [[clang::optnone]]
    void touch_jstring(const JString &s)
    {
      touch_cstr(s.c_str());
    }

    [[clang::optnone]]
    void touch_vec3(const Vec3<float> &v)
    {
      touch(v.x);
      touch(v.y);
      touch(v.z);
    }

    [[clang::optnone, gnu::noinline]]
    void verify_status(const EntityStatus *status)
    {
      if (!status)
      {
        return;
      }

      touch(status->bars.begin());
      touch(status->bars.end());
      touch(status->bars.max_ptr());
      for (auto bar = status->bars.begin(); bar != status->bars.end(); bar++)
      {
        if (auto data = bar->data)
        {
          touch(data->value);
          touch(data->display_time);
          if (auto config = data->config)
          {
            touch(config->id);
          }
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_movement(const MovementQueue *queue)
    {
      if (!queue)
      {
        return;
      }

      touch(queue->points.data());
      touch(queue->points.size());
      if (queue->points.data() && queue->points.size() > 0)
      {
        touch(queue->points.data()[0].scene_x);
        touch(queue->points.data()[0].scene_y);
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_named_entity(const NamedEntity *entity)
    {
      if (!entity)
      {
        return;
      }

      touch(entity->parent);
      touch(static_cast<uint8_t>(entity->type));
      touch(entity->terrain);
      touch(entity->server_index);
      touch_jstring(entity->name);
      touch_vec3(entity->position);
      touch(entity->animation_queue.begin());
      touch(entity->animation_queue.end());
      touch(entity->animation_queue.size());
      if (entity->animation_queue.begin() != entity->animation_queue.end())
      {
        touch(*entity->animation_queue.front());
      }

      verify_movement(entity->movement_queue);
      verify_status(entity->status);
    }

    [[clang::optnone, gnu::noinline]]
    void verify_player(const Player *player)
    {
      if (!player)
      {
        return;
      }

      verify_named_entity(player);
      touch(player->model);
      touch(player->combat_level);
      touch(player->skill_level);
    }

    [[clang::optnone, gnu::noinline]]
    void verify_npc(const Npc *npc)
    {
      if (!npc)
      {
        return;
      }

      verify_named_entity(npc);
      touch(npc->cache_id);
      touch(npc->visible_level);
    }

    [[clang::optnone, gnu::noinline]]
    void verify_local_player(const LocalPlayer *local_player, const PlayerUpdateCache *player_cache)
    {
      if (!local_player)
      {
        return;
      }

      touch(local_player->entity_list_index);
      touch_cstr(local_player->name);
      if (!player_cache)
      {
        return;
      }

      auto idx = local_player->entity_list_index;
      if (idx < 0 || idx >= static_cast<int32_t>(player_cache->updates.size()))
      {
        return;
      }

      if (auto update = *player_cache->updates.reference(static_cast<size_t>(idx)))
      {
        verify_player(update->player);
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_players(const PlayerUpdateCache *cache)
    {
      if (!cache)
      {
        return;
      }

      touch(cache->updates.begin());
      touch(cache->updates.end());
      touch(cache->updates.max_ptr());
      for (auto update = cache->updates.begin(); update != cache->updates.end(); update++)
      {
        if (auto entry = *update)
        {
          verify_player(entry->player);
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_npcs(const NpcUpdateCache *cache)
    {
      if (!cache)
      {
        return;
      }

      touch(cache->npcs);
      touch(cache->size);
      touch(cache->valid_count);
      if (!cache->npcs)
      {
        return;
      }

      for (uint64_t i = 0; i < cache->size; i++)
      {
        if (auto update = cache->npcs[i])
        {
          touch(update->id);
          touch(update->entity_header);
          verify_npc(update->npc);
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_items(const ItemCache *cache)
    {
      if (!cache)
      {
        return;
      }

      touch(cache->containers.begin());
      touch(cache->containers.end());
      touch(cache->containers.max_ptr());
      for (auto container = cache->containers.begin(); container != cache->containers.end(); container++)
      {
        touch(container->id);
        touch(container->items.begin());
        touch(container->items.end());
        for (auto item = container->items.begin(); item != container->items.end(); item++)
        {
          touch(item->id);
          touch(item->amount);
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_world_settings(const WorldSettingCache *cache)
    {
      if (!cache || !cache->vars)
      {
        return;
      }

      touch(cache->count);
      for (uint32_t i = 0; i < cache->count; i++)
      {
        auto setting = cache->vars[i];
        constexpr uint32_t max_chain = 4096;
        for (uint32_t link = 0; setting && link < max_chain; link++)
        {
          touch(setting->id);
          touch(setting->body.value);
          touch(setting->body.initialized);
          setting = setting->body.next;
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_stats(const VariableCache *cache)
    {
      if (!cache)
      {
        return;
      }

      auto local_vars = cache->local_player_variables;
      if (!local_vars)
      {
        return;
      }

      touch(local_vars->run_energy);
      touch(local_vars->stat_count);
      if (!local_vars->stats)
      {
        return;
      }

      for (uint32_t i = 0; i < local_vars->stat_count; i++)
      {
        auto stat = local_vars->stats[i];
        touch(stat.experience);
        touch(stat.current_level);
        touch(stat.max_level);
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_social(const SocialCache *cache)
    {
      if (!cache)
      {
        return;
      }

      for (auto friend_ = cache->friends.begin(); friend_ != cache->friends.end(); friend_++)
      {
        touch_jstring(friend_->name);
        touch_jstring(friend_->previous_name);
        touch_jstring(friend_->world);
      }

      for (auto ignored = cache->ignored.begin(); ignored != cache->ignored.end(); ignored++)
      {
        touch_jstring(ignored->name);
        touch_jstring(ignored->previous_name);
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_menu(const Menu *menu)
    {
      if (!menu)
      {
        return;
      }

      touch(menu->is_open);
      touch(menu->menu_options.begin());
      touch(menu->menu_options.end());
      for (auto option = menu->menu_options.begin(); option != menu->menu_options.end(); option++)
      {
        touch(option->tag);
        if (auto body = option->body)
        {
          touch_jstring(body->option_text);
          touch_jstring(body->entity_text);
          if (auto action = body->action_template)
          {
            touch(action->engine);
            touch(action->handler);
            touch(action->id);
            touch(static_cast<uint32_t>(action->type));
          }
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_widgets(const WidgetCache *cache)
    {
      if (!cache)
      {
        return;
      }

      touch(cache->widget_item_selected);
      touch(cache->c.begin());
      touch(cache->c.end());
      for (auto widget_001 = cache->c.begin(); widget_001 != cache->c.end(); widget_001++)
      {
        auto widget_002 = widget_001->widget_002;
        if (!widget_002)
        {
          continue;
        }

        touch(widget_002->id);
        for (auto widget_003 = widget_002->widgets_003.begin(); widget_003 != widget_002->widgets_003.end(); widget_003++)
        {
          if (auto widget = widget_003->widget)
          {
            touch(widget->parent_id);
            touch(widget->child_id);
            touch(widget->parent);
            touch(widget->x);
            touch(widget->y);
            touch(widget->width);
            touch(widget->height);
            touch(static_cast<uint8_t>(widget->get_type()));
          }
        }
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_world_node(WorldNode *node, int depth)
    {
      if (!node || depth > 128)
      {
        return;
      }

      touch_vec3(node->pos_a);
      touch_vec3(node->pos_b);
      touch_vec3(node->pos_c);
      touch_vec3(node->pos_avg);
      touch(static_cast<uint16_t>(node->flags));
      if (auto entity = node->entity)
      {
        touch(entity->parent);
        touch(static_cast<uint8_t>(entity->type));
        touch(entity->terrain);
      }

      for (auto child = node->children.begin(); child != node->children.end(); child++)
      {
        verify_world_node(*child, depth + 1);
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_scene(const Scene001 *scene_001)
    {
      if (!scene_001)
      {
        return;
      }

      touch(scene_001->scene_index);
      touch(scene_001->scene_002.begin());
      touch(scene_001->scene_002.end());
      auto scene_002 = scene_001->scene_002.reference(static_cast<size_t>(scene_001->scene_index));
      if (!scene_002)
      {
        return;
      }

      auto scene_003 = scene_002->scene_003;
      if (!scene_003)
      {
        return;
      }

      touch(scene_003->projection_matrix.flat[0]);
      touch(scene_003->projection_matrix.flat[15]);
      verify_world_node(scene_003->world_root, 0);
    }

    [[clang::optnone, gnu::noinline]]
    void verify_cache(Cache001 *cache)
    {
      if (!cache)
      {
        return;
      }

      touch(cache->indices);
      if (auto ws_index = cache->get_world_settings_index())
      {
        touch(ws_index->current_size);
        touch(ws_index->max_size);
        touch(ws_index->inner);
        if (ws_index->inner)
        {
          touch(ws_index->inner->size);
        }
      }

      if (!cache->indices)
      {
        return;
      }

      auto items = cache->indices[static_cast<uint8_t>(CacheIndexOrdinal::items)];
      if (!items)
      {
        return;
      }

      touch(items->current_size);
      touch(items->max_size);
      touch(items->inner);
      if (items->inner)
      {
        touch(items->inner->size);
      }
    }

    [[clang::optnone, gnu::noinline]]
    void verify_linux(const Globals *globals)
    {
      if (!globals)
      {
        return;
      }

      touch(globals->heap);
      touch(globals->linux_001);
      touch(NRS.sdl_window());
    }
  } // namespace

  [[clang::optnone, gnu::noinline]]
  void EngineTickHook::verify()
  {
    auto globals = RS.get_globals().unwrap();
    if (!globals)
    {
      return;
    }

    auto engine = globals->engine;
    if (!engine)
    {
      return;
    }

    touch(engine->time);
    touch(static_cast<uint32_t>(engine->state));
    touch(engine->world_a);

    verify_local_player(engine->local_player, engine->player_update_cache);
    verify_players(engine->player_update_cache);
    verify_npcs(engine->npc_update_cache);
    verify_items(engine->item_cache);
    verify_world_settings(&engine->world_settings);
    verify_stats(engine->variable_cache);
    verify_social(engine->social_cache);
    verify_menu(engine->menu);
    verify_widgets(engine->widget_cache);
    verify_scene(engine->scene_001);
    verify_cache(engine->cache);
    verify_linux(globals);
  }

  void EngineTickHook::tick_ui(Engine *engine)
  {
    if (RS.ui_visible)
    {
      if (engine->state == GameState::lobby_screen || engine->state == GameState::in_game)
      {
        if (RS.dom_node_item_containers->visible)
        {
          RS.dom_node_item_containers->update();
        }
      }

      if (engine->state == GameState::in_game)
      {
        if (RS.dom_node_stats->visible)
        {
          RS.dom_node_stats->update();
        }

        if (RS.dom_node_npcs->visible)
        {
          RS.dom_node_npcs->update();
        }

        if (RS.dom_node_players->visible)
        {
          RS.dom_node_players->update();
        }

        if (RS.dom_node_world_settings->visible)
        {
          RS.dom_node_world_settings->update();
        }
      }
    }
  }

  void EngineTickHook::tick_imgui()
  {
    auto swap_buffers_hook = RS.hook_manager->view_hook<EglSwapBuffersHook>("egl_swap_buffers");
    if (swap_buffers_hook && !swap_buffers_hook->is_first_run)
    {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui::NewFrame();
      RS.developer_overlay.render();

      ImGui::Render();

      auto &io = ImGui::GetIO();
      RS.imgui_want_capture_mouse.store(io.WantCaptureMouse, std::memory_order_relaxed);
      RS.imgui_want_capture_keyboard.store(io.WantCaptureKeyboard, std::memory_order_relaxed);
    }
  }

  void EngineTickHook::tick_stats()
  {
    if (RS.stats_timer.check(std::chrono::seconds(5)))
    {
      auto item_container_dom_nodes_created_recent = RS.stats.item_container_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
      auto item_container_dom_nodes_removed_recent = RS.stats.item_container_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

      auto item_dom_nodes_created_recent = RS.stats.item_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
      auto item_dom_nodes_removed_recent = RS.stats.item_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

      auto player_dom_nodes_created_recent = RS.stats.player_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
      auto player_dom_nodes_removed_recent = RS.stats.player_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

      auto npc_dom_nodes_created_recent = RS.stats.npc_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
      auto npc_dom_nodes_removed_recent = RS.stats.npc_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

      auto object_dom_nodes_created_recent = RS.stats.object_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
      auto object_dom_nodes_removed_recent = RS.stats.object_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

      LOG(INFO, " -> Stats");
      LOG(INFO, " -> New Item Container DOM Nodes: " << RS.stats.item_container_dom_nodes_created << "/" << item_container_dom_nodes_created_recent << "/" << item_container_dom_nodes_removed_recent);

      LOG(INFO, " -> New Item DOM Nodes: " << RS.stats.item_dom_nodes_created << "/" << item_dom_nodes_created_recent << "/" << item_dom_nodes_removed_recent);

      LOG(INFO, " -> New Player DOM Nodes: " << RS.stats.player_dom_nodes_created << "/" << player_dom_nodes_created_recent << "/" << player_dom_nodes_removed_recent);

      LOG(INFO, " -> New NPC DOM Nodes: " << RS.stats.npc_dom_nodes_created << "/" << npc_dom_nodes_created_recent << "/" << npc_dom_nodes_removed_recent);

      LOG(INFO, " -> New Object DOM Nodes: " << RS.stats.object_dom_nodes_created << "/" << object_dom_nodes_created_recent << "/" << object_dom_nodes_removed_recent);

      LOG(INFO, " -> Push UI State Time: " << RS.stats.push_ui_state_stopwatch.check_ms());
      LOG(INFO, " -> Render UI Time: " << RS.stats.render_ui_stopwatch.check_ms());
    }
  }

  void EngineTickHook::watch_item_changes(Engine *engine)
  {
    auto get_cached_container = [this](uint32_t idx) -> ItemContainerCache &
    {
      return this->cached_containers.try_emplace(idx).first->second;
    };

    std::vector<ItemChangedArgs> changes;
    if (auto cache = engine->item_cache)
    {
      for (auto it = cache->containers.begin(); it != cache->containers.end(); it++)
      {
        auto container_id = it->id;
        std::vector<Item> snapshot(it->items.begin(), it->items.end());

        auto &container = get_cached_container(container_id);
        if (snapshot.size() > container.items.size())
        {
          container.items.resize(snapshot.size());
        }

        for (size_t slot = 0; slot < snapshot.size(); slot++)
        {
          auto &container_item = container.items[slot];
          const auto &item = snapshot[slot];
          if (container_item.id == item.id && container_item.amount == item.amount)
          {
            continue;
          }

          auto delta = 0;
          if (container_item.id == item.id || container_item.id == -1)
          {
            delta = item.amount - container_item.amount;
          }

          changes.push_back(ItemChangedArgs{ container_id, static_cast<uint32_t>(slot), container_item.id, container_item.amount, item.id, item.amount, delta });
          container_item = item;
        }
      }
    }

    for (auto &change : changes)
    {
      auto event = ItemChangedEvent(change.id, change.slot, change.old_id, change.old_amount, change.new_id, change.new_amount, change.stack_delta);
      RS.event_bus.dispatch(ItemChangedEvent::specific_id(), &event);
    }
  }

  void EngineTickHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);
    auto swap_buffers_hook = RS.hook_manager->view_hook<EglSwapBuffersHook>("egl_swap_buffers");
    if (!plugins_loaded && swap_buffers_hook && !swap_buffers_hook->is_first_run)
    {
      LOG(INFO, "Loading plugins...");
      RS.plugin_manager.init();
      RS.plugin_manager.load_all(RS.resolve_configuration("plugins/"));

      plugins_loaded = true;
    }

    auto engine = reinterpret_cast<Engine *>(CPU_FIRST_ARG(cpu_state));
    RS.ui_locked_nr([this, engine]()
    {
      tick_ui(engine);
      tick_imgui();
    });

    tick_stats();

    auto event = EngineTickEvent(engine);
    RS.event_bus.dispatch(EngineTickEvent::specific_id(), &event);

    watch_item_changes(engine);

    if (RS.ui->check_verify())
    {
      verify();
    }

    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
        engine,
        CPU_FIRST_FARG(cpu_state)));
  }
} // namespace crs
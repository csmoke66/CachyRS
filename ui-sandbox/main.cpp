#include "log.h"
#include "rml_ui.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <chrono>
#include <cmath>
#include <format>
#include <memory>
#include <string>
#include <thread>

namespace
{
  using namespace crs;

  constexpr int kWidth = 1280;
  constexpr int kHeight = 800;

  std::shared_ptr<DomNode> make_node(const std::shared_ptr<DomTree> &tree, const std::string &id, const std::string &type)
  {
    return std::make_shared<DomNode>(tree, id, type);
  }

  void seed_dom(RmlUserInterface &ui)
  {
    auto tree = std::static_pointer_cast<DomTree>(ui.shared_from_this());

    auto world = make_node(tree, "world", "world");
    {
      auto name = std::make_unique<StringDomValue>("name", "Lumbridge");
      name->mark_inlined();
      world->add_value(std::move(name));
      world->add_value(std::make_unique<Int32DomValue>("plane", 0));
      world->add_value(std::make_unique<UInt32DomValue>("tick", 42));
    }

    auto players = make_node(tree, "players", "players");
    for (int i = 0; i < 4; ++i)
    {
      auto player = make_node(tree, std::format("player_{}", i), "player");
      auto pname = std::make_unique<StringDomValue>("name", std::format("Player{}", i));
      pname->mark_inlined();
      player->add_value(std::move(pname));
      player->add_value(std::make_unique<Int32DomValue>("combat", 3 + i * 10));
      player->add_value(std::make_unique<Int32DomValue>("x", 3200 + i));
      player->add_value(std::make_unique<Int32DomValue>("y", 3200 - i));
      players->add_child(player);
    }

    auto npcs = make_node(tree, "npcs", "npcs");
    for (const char *npc_name : { "Man", "Woman", "Goblin", "Giant rat" })
    {
      auto npc = make_node(tree, sanitize_dom_id(npc_name), "npc");
      auto n = std::make_unique<StringDomValue>("name", npc_name);
      n->mark_inlined();
      npc->add_value(std::move(n));
      npc->add_value(std::make_unique<Int32DomValue>("id", static_cast<int>(std::hash<std::string_view>{}(npc_name) % 1000)));
      npcs->add_child(npc);
    }

    auto widgets = make_node(tree, "widgets", "widgets");
    auto root_w = make_node(tree, "widget_820", "widget");
    {
      auto path = std::make_unique<StringDomValue>("path", "820");
      path->mark_inlined();
      root_w->add_value(std::move(path));
      root_w->add_value(std::make_unique<StringDomValue>("type", "layer"));
      root_w->add_value(std::make_unique<StringDomValue>("text", ""));
    }
    for (int child : { 2, 14, 18, 24, 28 })
    {
      auto w = make_node(tree, std::format("widget_820_{}", child), "widget");
      auto path = std::make_unique<StringDomValue>("path", std::format("820/{}", child));
      path->mark_inlined();
      w->add_value(std::move(path));
      w->add_value(std::make_unique<StringDomValue>("text", child == 24 ? "Play Now" : std::format("Child {}", child)));
      w->add_value(std::make_unique<StringDomValue>("menu", child == 24 ? "Play Now" : ""));
      root_w->add_child(w);
    }
    widgets->add_child(root_w);

    world->add_child(players);
    world->add_child(npcs);
    world->add_child(widgets);
    ui.add_dom_node(world);
  }

  void tick_dom(RmlUserInterface &ui, uint32_t frame)
  {
    if (auto *world = ui.root_dom_node->find_dom_node("world"))
    {
      world->set_value<UInt32DomValue>("tick", frame);
    }

    if (auto *p0 = ui.root_dom_node->find_dom_node("player_0"))
    {
      p0->set_value<Int32DomValue>("x", 3200 + static_cast<int>(std::sin(frame * 0.05f) * 5.f));
    }
  }

  void feed_logs(uint32_t frame)
  {
    if (frame % 90 == 0)
    {
      LOG(INFO, "sandbox heartbeat frame=" << frame);
    }
    if (frame % 180 == 30)
    {
      LOG(WARNING, "simulated warning: inventory nearly full");
    }
    if (frame % 240 == 60)
    {
      LOG(ERROR, "simulated error: failed to pathfind");
    }
    if (frame % 45 == 10)
    {
      // Same content on purpose — exercises Unity-style collapse bubbles.
      LOG(PLUGIN, "auto-miner: waiting for lobby");
    }
    if (frame % 120 == 15)
    {
      LOG(DEBUG, "debug detail frame=" << frame);
    }
  }
} // namespace

int main(int argc, char **argv)
{
  using namespace crs;

  const char *config_override = nullptr;
  for (int i = 1; i < argc; ++i)
  {
    const std::string arg = argv[i];
    if (arg == "--config" && i + 1 < argc)
    {
      config_override = argv[++i];
    }
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
  {
    LOG(ERROR, "SDL_Init failed: " << SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_Window *window = SDL_CreateWindow(
      "CachyRS UI Sandbox",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      kWidth,
      kHeight,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

  if (!window)
  {
    LOG(ERROR, "SDL_CreateWindow failed: " << SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_GLContext gl = SDL_GL_CreateContext(window);
  if (!gl)
  {
    LOG(ERROR, "SDL_GL_CreateContext failed: " << SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_GL_MakeCurrent(window, gl);
  SDL_GL_SetSwapInterval(1);

  auto ui = std::make_shared<RmlUserInterface>();
  ui->pre_init();
  ui->set_widget_pick_handler([](WidgetPickMode mode)
  {
    LOG(INFO, "widget pick armed mode=" << (mode == WidgetPickMode::all ? "all" : "menu"));
  });

  std::string config_dir;
  if (config_override)
  {
    config_dir = config_override;
    if (!config_dir.empty() && config_dir.back() != '/')
    {
      config_dir.push_back('/');
    }
  }
  else if (char *base = SDL_GetBasePath())
  {
    config_dir = base;
    SDL_free(base);
  }

  ui->init("sandbox", config_dir, window, kWidth, kHeight);
  seed_dom(*ui);
  ui->show_main_tab("debug_content");
  LOG(INFO, "UI sandbox ready — config=" << config_dir);

  bool running = true;
  uint32_t frame = 0;
  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
      }
      else if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
      {
        running = false;
      }

      ui->process(&event);
    }

    feed_logs(frame);
    tick_dom(*ui, frame);
    if (ui->root_dom_node)
    {
      ui->build_dom_node(ui->root_dom_node.get());
    }

    // crs_ui intentionally skips framebuffer clear (game overlay). Sandbox owns the window.
    int drawable_w = 0;
    int drawable_h = 0;
    SDL_GL_GetDrawableSize(window, &drawable_w, &drawable_h);
    glViewport(0, 0, drawable_w, drawable_h);
    glClearColor(0.12f, 0.12f, 0.12f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ui->render();
    SDL_GL_SwapWindow(window);

    ++frame;
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  ui.reset();
  SDL_GL_DeleteContext(gl);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

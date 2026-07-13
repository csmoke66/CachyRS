#pragma once
#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

#include "math.h"

enum class UserVariable : uint16_t
{
    player_overlay
};

struct UserInterfaceItem
{
    int32_t id;
    int32_t amount;
};

struct UserInterfaceItemContainer
{
    uint32_t id;
    std::vector<UserInterfaceItem> items;
};

struct UserInterfaceState
{
    Matrix4x4 projection_matrix;
    std::vector<UserInterfaceItemContainer> item_containers;
};

class UserInterface
{
public:
    virtual ~UserInterface();

public:
    virtual void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) = 0;
    virtual void reload();

public:
    virtual void process(SDL_Event *event) = 0;
    virtual bool wants_input() = 0;

public:
    virtual void render() = 0;

public:
    virtual void propagate(const UserInterfaceState &state) = 0;

public:
    virtual bool get_bool(UserVariable var) = 0;
    virtual void set_bool(UserVariable var, bool b) = 0;
};

class CachySystemInterface : public Rml::SystemInterface
{
public:
    bool LogMessage(Rml::Log::Type type, const Rml::String &message);
};

class RmlUserInterface : public UserInterface
{
private:
    std::string version;
    std::string config_folder;
    SDL_Window *sdl_window;

private:
    bool wants_input_last = false;

private:
    CachySystemInterface system_interface;
    Rml::Context *context = nullptr;
    Rml::ElementDocument *root_document = nullptr;

    Rml::Element *selected_tab_button = nullptr;
    Rml::Element *selected_content = nullptr;

    Rml::Element *home_tab_button = nullptr;
    Rml::Element *home_content = nullptr;

    Rml::Element *plugins_tab_button = nullptr;
    Rml::Element *plugins_content = nullptr;

    Rml::Element *debug_tab_button = nullptr;
    Rml::Element *debug_content = nullptr;
    Rml::Element *debug_projection = nullptr;
    Rml::Element *debug_item_containers = nullptr;

private:
    bool player_overlay_on = false;

private:
    void load_fonts();

public:
    void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) override;
    void reload() override;

public:
    void process(SDL_Event *event) override;
    bool wants_input() override;

public:
    void render() override;

public:
    void propagate(const UserInterfaceState &state) override;

public:
    bool get_bool(UserVariable var) override;
    void set_bool(UserVariable var, bool b) override;
};
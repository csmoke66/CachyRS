#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <functional>

#include <SDL2/SDL.h>

#include "math.h"
#include "dom.h"

namespace crs
{
    enum class ComponentType
    {
        container,
        label,
        checkbox,
        button,
        hr,
        line,
    };

    struct Component
    {
    public:
        uint64_t id;
        std::vector<Component*> children;

    public:
        Component(uint64_t id)
        {
            this->id = id;
        }

    public:
        Component* get_child(size_t id)
        {
            if (this->id == id)
            {
                return this;
            }

            for (auto c : children)
            {
                if (auto cc = c->get_child(id))
                {
                    return cc;
                }
            }

            return nullptr;
        }
    };

    class UserInterface
    {
    public:
        virtual ~UserInterface();

    public:
        virtual void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) = 0;
        virtual void reload();
        virtual void add_reload_callback(std::function<void()> function);
        
    public:
        virtual void process(SDL_Event *event) = 0;
        virtual bool wants_input() = 0;

    public:
        virtual uint64_t allocate_tab(const std::string& name) = 0;
        virtual uint64_t allocate_component(ComponentType type, uint64_t parent_id) = 0;
        virtual void update_component_text(uint64_t component_id, std::string text) = 0;
        virtual bool is_component_checked(uint64_t component_id) = 0;

    public:
        virtual void render() = 0;
    };
}
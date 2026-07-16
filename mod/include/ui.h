#pragma once
#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>

#include "math.h"
#include "dom.h"

namespace crs
{
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
    };

    class CachySystemInterface : public Rml::SystemInterface
    {
    public:
        bool LogMessage(Rml::Log::Type type, const Rml::String &message);
    };

}
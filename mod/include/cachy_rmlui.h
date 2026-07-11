#pragma once
#include <RmlUi/Core.h>

class CachySystemInterface : public Rml::SystemInterface
{
public:
    bool LogMessage(Rml::Log::Type type, const Rml::String &message);
};

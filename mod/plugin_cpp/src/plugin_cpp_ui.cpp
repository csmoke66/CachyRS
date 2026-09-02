#include "plugin_cpp.h"

namespace crs
{
    ApiLabel::ApiLabel(PluginApi api, uint64_t id)
    {
        this->api = api;
        this->id = id;
    }

    ApiLabel::ApiLabel(const ApiLabel &o)
    {
        this->api = o.api;
        this->id = o.id;
    }

    ApiHr::ApiHr(PluginApi api, uint64_t id)
    {
        this->api = api;
        this->id = id;
    }

    ApiHr::ApiHr(const ApiHr &o)
    {
        this->api = o.api;
        this->id = o.id;
    }

    ApiCheckBox::ApiCheckBox()
    {
        this->id = (uint64_t)-1;
        memset(&this->api, 0, sizeof(this->api));
    }

    ApiCheckBox::ApiCheckBox(PluginApi api, uint64_t id)
    {
        this->api = api;
        this->id = id;
    }

    ApiCheckBox::ApiCheckBox(const ApiCheckBox &o)
    {
        this->api = o.api;
        this->id = o.id;
    }

    bool ApiCheckBox::is_checked()
    {
        return api.ui_is_component_checked(id);
    }
}
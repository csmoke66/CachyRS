#include "rml_ui.h"

namespace crs
{
    Rml::DecoratorDataHandle VisibilityTrackerDecorator::GenerateElementData(Rml::Element *element, Rml::BoxArea element_data) const
    {
        return 1;
    }

    void VisibilityTrackerDecorator::ReleaseElementData(Rml::DecoratorDataHandle element_data) const
    {
        
    }

    bool VisibilityTrackerDecorator::IsElementOnScreen(Rml::Element *element, Rml::Context *context) const
    {
        if (!element || !context)
            return false;

        auto box_size = element->GetBox().GetSize(Rml::BoxArea::Border);
        if (box_size.x <= 0.0f || box_size.y <= 0.0f)
        {
            return false;
        }

        auto absolute_pos = element->GetAbsoluteOffset(Rml::BoxArea::Border);
        auto context_size = context->GetDimensions();

        bool is_outside = (absolute_pos.x + box_size.x < 0.0f) ||
                          (absolute_pos.y + box_size.y < 0.0f) ||
                          (absolute_pos.x > static_cast<float>(context_size.x)) ||
                          (absolute_pos.y > static_cast<float>(context_size.y));

        return !is_outside;
    }

    void VisibilityTrackerDecorator::RenderElement(Rml::Element *element, Rml::DecoratorDataHandle element_data) const
    {
        if (IsElementOnScreen(element, element->GetContext()))
        {
            element->SetAttribute("render_frame", parent->get_render_frame());
        }
        else
        {
            element->SetAttribute("render_frame", 0);
        }
    }

    Rml::SharedPtr<Rml::Decorator> VisibilityTrackerInstancer::InstanceDecorator(
        const Rml::String &name,
        const Rml::PropertyDictionary &properties,
        const Rml::DecoratorInstancerInterface &instancer_interface)
    {
        auto r = Rml::MakeShared<VisibilityTrackerDecorator>();
        r->parent = parent;
        return r;
    }
}
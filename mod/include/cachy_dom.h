#pragma once
#include "dom.h"

namespace crs
{
    class CachyDomTreeListener : public DomTreeListener
    {
        void on_click(std::shared_ptr<DomNode> node) override;
    };
}
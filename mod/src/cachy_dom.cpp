#include "cachy_dom.h"
#include "game_dom.h"

#include "cachy.h"

namespace crs
{
    void CachyDomTreeListener::on_click(std::shared_ptr<DomNode> node)
    {
        if (std::dynamic_pointer_cast<PlayersDomNode>(node))
        {
            RS.developer_overlay.player_overlay_on = true;
        }
        else
        {
            RS.developer_overlay.player_overlay_on = false;
        }
    }
}
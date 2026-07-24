#include "cachy_dom.h"
#include "game_dom.h"

#include "cachy.h"

namespace crs
{
    void CachyDomTreeListener::on_click(std::shared_ptr<DomNode> node)
    {
        auto players_dom_node = std::dynamic_pointer_cast<PlayersDomNode>(node);
        auto player_dom_node = std::dynamic_pointer_cast<PlayerDomNode>(node);
        if (!!players_dom_node || !!player_dom_node)
        {
            RS.developer_overlay.player_overlay_on = true;
            if (!!player_dom_node)
            {
                RS.developer_overlay.player_overlay_target = player_dom_node->player;
            }
            else
            {
                RS.developer_overlay.player_overlay_target = nullptr;
            }
        }
        else
        {
            RS.developer_overlay.player_overlay_on = false;
        }
    }
}
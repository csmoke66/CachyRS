#include "reversed/reversed.h"

namespace crs
{
    Item::Item() : id(0), amount(0)
    {

    }

    Item::Item(const Item &o) : id(o.id), amount(o.amount)
    {
        
    }
}
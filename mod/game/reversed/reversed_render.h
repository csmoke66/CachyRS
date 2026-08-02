#pragma once

#pragma pack(push, 1)
struct ModelItem
{
    // 0x0
    uint32_t unknown;
    // 0x4
    uint32_t item_id;
    // 0x8
};

struct Model
{
    // 0x0
    PAD(0x28);
    // 0x28
    JArray<ModelItem> model_items;
    // 0x38
};
#pragma pack(pop)
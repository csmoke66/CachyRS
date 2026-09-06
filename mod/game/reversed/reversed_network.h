#pragma once

#pragma pack(push, 1)
struct Packet
{
    // 0x0
    uint32_t opcode;
    // 0x4
    PAD(0x14);
    // 0x18
    uint8_t* data;
    // 0x20
    size_t size;
};

struct PacketContainer
{
    // 0x0
    PAD(0x8);
    // 0x8
    Packet* packet;
};
#pragma pack(pop)
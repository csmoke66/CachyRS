#pragma once
#include <type_traits>

inline constexpr WorldNodeFlag operator~(WorldNodeFlag lhs) 
{
    return (WorldNodeFlag)(
        ~(std::underlying_type_t<WorldNodeFlag>)(lhs)
    );
}

inline constexpr WorldNodeFlag operator&(WorldNodeFlag lhs, WorldNodeFlag rhs) 
{
    return (WorldNodeFlag)(
        (std::underlying_type_t<WorldNodeFlag>)(lhs) & 
        (std::underlying_type_t<WorldNodeFlag>)(rhs)
    );
}

inline constexpr WorldNodeFlag& operator&=(WorldNodeFlag& lhs, WorldNodeFlag rhs) 
{
    lhs = lhs & rhs;
    return lhs;
}

inline constexpr WorldNodeFlag operator|(WorldNodeFlag lhs, WorldNodeFlag rhs) 
{
    return (WorldNodeFlag)(
        (std::underlying_type_t<WorldNodeFlag>)(lhs) |
        (std::underlying_type_t<WorldNodeFlag>)(rhs)
    );
}

inline constexpr WorldNodeFlag& operator|=(WorldNodeFlag& lhs, WorldNodeFlag rhs) 
{
    lhs = lhs | rhs;
    return lhs;
}
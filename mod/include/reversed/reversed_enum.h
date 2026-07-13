#pragma once
#include <cstdint>

enum class EntityType : uint8_t
{
	npc = 1,
	player = 2,
	ground_item = 3,
	effect = 4,
	object1 = 0,
	object2 = 12
};

enum class GameState : uint32_t
{
	login_screen = 10,
	lobby_screen = 20,
	in_game = 30
};

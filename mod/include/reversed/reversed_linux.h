#pragma once
#include "reversed_manual_base.h"

#include <SDL2/SDL.h>

#pragma pack(push, 1)
struct Linux005
{
	// 0x0
	PAD(0x28);
	// 0x28
	const SDL_Window* sdl_window;
};

struct Linux004
{
	// 0x0
	PAD(0x90);
	// 0x90
	const Linux005* linux_005;
};

struct Linux003
{
	// 0x0
	PAD(0x8);
	// 0x8
	const Linux004* linux_004;
};

struct Linux002
{
	// 0x0
	PAD(0x30);
	// 0x30
	const Linux003* linux_003;
};

struct Linux001
{
	// 0x0
	PAD(0x1b8);
	// 0x1b8
	const Linux002* linux_002;
};

#pragma pack(pop)
#pragma once
#include <EGL/egl.h>
#include <SDL2/SDL.h>
#include <cstdint>

#include "reversed_util.h"
#include "reversed_manual_base.h"

struct Linux005;
struct Linux004;
struct Linux003;
struct Linux002;
struct Linux001;
struct WindowState;
struct WorldNode;
struct Scene002;
struct Scene001;
struct WorldA;

enum class EntityType : uint8_t
{
	npc = 1,
	player = 2,
	ground_item = 3,
	effect = 4,
	object1 = 0,
	object2 = 12
};

#include "reversed_generated.h"

#pragma pack(push, 1)
struct Linux005
{
	// 0x0
	PAD(0x28);
	// 0x28
	SDL_Window* sdl_window;
};

struct Linux004
{
	// 0x0
	PAD(0x90);
	// 0x90
	Linux005* linux_005;
};

struct Linux003
{
	// 0x0
	PAD(0x8);
	// 0x8
	Linux004* linux_004;
};

struct Linux002
{
	// 0x0
	PAD(0x30);
	// 0x30
	Linux003* linux_003;
};

struct Linux001
{
	// 0x0
	PAD(0x1b8);
	// 0x1b8
	Linux002* linux_002;
};

struct WindowState
{
	// 0x0
	PAD(0x28);
	// 0x28
	SDL_Window* window;
};

struct WorldNode
{
	// 0x0
    PAD(0x50);
	// 0x50
	Vec3<float> max;
	// 0x5c
    PAD(0x24);
	// 0x80
	Vec3<float> min;
	// 0x8c
    PAD(0xAC);
	// 0x138
	JArray<WorldNode*> children;
	// 0x148
    PAD(0x58);
	// 0x1a0
	Entity* entity;
	// 0x1d8

public:
	__attribute__((always_inline)) inline Vec3<float> center()
	{
		return { min.x + (max.x - min.x), min.y + (max.y - min.y), min.z + (max.z - min.z) };
	}

	__attribute__((always_inline)) inline uint32_t tile_x()
	{
		return (uint32_t)(center().x / 512.f);
	}

	__attribute__((always_inline)) inline uint32_t tile_y()
	{
		return (uint32_t)(center().z / 512.f);
	}

	__attribute__((always_inline)) inline Vec2<uint32_t> tile()
	{
		return { tile_x(), tile_y() };
	}
};
static_assert(offsetof(WorldNode, children) == 0x138, INVALID_OFFSET);
static_assert(offsetof(WorldNode, entity) == 0x1a0, INVALID_OFFSET);

struct Scene002
{
    // 0x0
    PAD(0x8);
    // 0x8
    Scene003* scene_003;
};

struct Scene001
{
    // 0x0
    PAD(0x58);
    // 0x58
    JArray<Scene002> scene_002;
    // 0x68
    PAD(0x8);
    // 0x70
    int32_t scene_index;
};
static_assert(offsetof(Scene001, scene_index) == 0x70, INVALID_OFFSET);

struct WorldA
{

};
#pragma pack(pop)

typedef EGLBoolean (*FnEglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
typedef int (*FnSDL_PollEvent)(SDL_Event* event);
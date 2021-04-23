#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "universal.h"

// ============================================================================
// Tilemap

enum class TileType {
	NONE,
	GRASS,
	WATER,
	MOUNTAIN,
	HOUSE,
	FORT,
	MINE,
	RAIL,
	NUM_TYPES
};

struct Tile {
	TileType type;
};

struct Tilemap {
	int32_t width;
	int32_t height;
	Tile* tiles;
};

// End Tilemap
// ============================================================================
// UI System

enum class UIRectType {
	BOX,
	GAME,
	NUM_RECT_TYPES
};

struct UIRect {
	int32_t layer;
	int32_t x, y;
	int32_t w, h;

	UIRectType type;
	int32_t line_width;
};

struct UISystem {
	static const int32_t NUM_RECTS = 4;
	UIRect rects[NUM_RECTS];
};

// End UI System
// ============================================================================

struct GameState {
	int64_t game_tick;
	
	Tilemap tilemap;

	f32 x;
	f32 y;
	int32_t s;

	UISystem ui_system;
};

#endif

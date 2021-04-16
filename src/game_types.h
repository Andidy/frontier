#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "universal.h"

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

struct GameState {
	Tilemap tilemap;

	f32 x;
	f32 y;
	int32_t s;
};

#endif

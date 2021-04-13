#ifndef GAME_H
#define GAME_H

#include "universal.h"

#include "../libs/simplex.h"

enum class TileType {
	NONE,
	GRASS,
	WATER,
	MOUNTAIN,
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
};

void InitGameState(Memory* gameMemory);
void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt);

#endif

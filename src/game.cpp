#include "game.h"

void InitGameState(Memory* gameMemory) {
	GameState* gs = (GameState*)gameMemory->data;

	const int tilemap_width = 32;
	const int tilemap_height = 18;

	gs->tilemap = { tilemap_width, tilemap_height, NULL };
	gs->tilemap.tiles = (Tile*)malloc(sizeof(Tile) * tilemap_width * tilemap_height);
	for (int y = 0; y < tilemap_height; y++) {
		for (int x = 0; x < tilemap_width; x++) {
			gs->tilemap.tiles[x + tilemap_width * y].type = TileType::GRASS;

			if (10 < x && x < 15 && 10 < y && y < 17) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::WATER;
			}

			if (x == 4 && 3 < y && y < 6) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::MOUNTAIN;
			}
		}
	}
}

void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt) {
	// GameState* gameState = (GameState*)gameMemory->data;

	// ========================================================================
	// Camera Update

	// end Camera Update
	// ========================================================================
}
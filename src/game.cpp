#include "game.h"

void InitGameState(Memory* gameMemory) {
	GameState* gs = (GameState*)gameMemory->data;

	gs->tilemap = { 32, 18, NULL };
	gs->tilemap.tiles = (Tile*)malloc(sizeof(Tile) * 32 * 18);
	for (int y = 0; y < 18; y++) {
		for (int x = 0; x < 32; x++) {
			gs->tilemap.tiles[x + 32 * y].type = TileType::GRASS;

			if (10 < x && x < 15 && 10 < y && y < 17) {
				gs->tilemap.tiles[x + 32 * y].type = TileType::WATER;
			}

			if (x == 4 && 3 < y && y < 6) {
				gs->tilemap.tiles[x + 32 * y].type = TileType::MOUNTAIN;
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
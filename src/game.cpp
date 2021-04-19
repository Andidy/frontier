#include "game.h"

void InitGameState(Memory* gameMemory) {
	GameState* gs = (GameState*)gameMemory->data;

	gs->x = 0;
	gs->y = 0;
	gs->s = 1;

	const int tilemap_width = 200;
	const int tilemap_height = 100;

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

			if (x == 5 && 3 < y && y < 6) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::MINE;
			}

			if (x == 16 && y == 3) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::FORT;
			}

			if (15 < x && x < 19 && 3 < y && y < 6) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::HOUSE;
			}

			if (x == 18 && 5 < y && y < 15) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::RAIL;
			}
		}
	}
}

void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt) {
	GameState* gs = (GameState*)gameMemory->data;

	// ========================================================================
	// Camera Update

	KeyboardState key = gameInput->keyboard;

	const f32 speed = 1.0f;
	if (keyDown(key.a)) {
		gs->x -= dt * speed;
	}
	if (keyDown(key.d)) {
		gs->x += dt * speed;
	}
	if (keyDown(key.w)) {
		gs->y -= dt * speed;
	}
	if (keyDown(key.s)) {
		gs->y += dt * speed;
	}

	if (keyReleased(key.r)) {
		gs->x /= gs->s;
		gs->y /= gs->s;
		gs->s += 1;
		if (gs->s > 8) gs->s = 8;
		gs->x *= gs->s;
		gs->y *= gs->s;
	}
	if (keyReleased(key.f)) {
		gs->x /= gs->s;
		gs->y /= gs->s;
		gs->s -= 1;
		if (gs->s < 1) gs->s = 1;
		gs->x *= gs->s;
		gs->y *= gs->s;
	}

	if (gs->x < 0.0f) gs->x = 0.0f;
	if (gs->y < 0.0f) gs->y = 0.0f;
	if (gs->x >= (200.0f * 32.0f * gs->s - 1024.0f)) gs->x = (200.0f * 32.0f * gs->s - 1024.0f);
	if (gs->y >= (100.0f * 32.0f * gs->s - 576.0f)) gs->y = (100.0f * 32.0f * gs->s - 576.0f);

	// end Camera Update
	// ========================================================================
}
#include "game.h"

extern CycleCounter global_cycle_counter;

// ============================================================================
// UI System

int32_t UIClick(UISystem* ui_system, int32_t x, int32_t y) {
	
	bool hits[ui_system->NUM_RECTS];

	for (int i = 0; i < ui_system->NUM_RECTS; i++) {
		hits[i] = false;
		if (ui_system->rects[i].x < x && x < (ui_system->rects[i].x + ui_system->rects[i].w) &&
			ui_system->rects[i].y < y && y < (ui_system->rects[i].y + ui_system->rects[i].h)) {
			hits[i] = true;
		}
	}

	int32_t highest_layer = -1;
	int32_t rect_hit = -1;
	for (int i = 0; i < ui_system->NUM_RECTS; i++) {
		if (hits[i]) {
			highest_layer = (ui_system->rects[i].layer > highest_layer) ? ui_system->rects[i].layer : highest_layer;
			rect_hit = i;
		}
	}

	return rect_hit;
}

void ScreenToTile(int scr_x, int scr_y, int rect_x, int rect_y, int view_x, int view_y, int tile_width, int tile_height, int32_t* out_x, int32_t* out_y) {
	*(out_x) = (scr_x - rect_x + view_x) / tile_width;
	*(out_y) = (scr_y - rect_y + view_y) / tile_height;
}

void TileToScreen(int world_x, int world_y, int rect_x, int rect_y, int view_x, int view_y, int tile_width, int tile_height, int32_t* out_x, int32_t* out_y) {
	*(out_x) = (world_x * tile_width) - view_x + rect_x;
	*(out_y) = (world_y * tile_height) - view_y + rect_y;
}

// End UI System
// ============================================================================
// Game Core

void InitGameState(Memory* gameMemory) {
	GameState* gs = (GameState*)gameMemory->data;

	gs->game_tick = 0;

	gs->x = 0;
	gs->y = 0;
	gs->s = 1;
	gs->selected_unit = -1;

	gs->ui_system.rects[0] = { 0, 256, 192, (32 * 40) - 256, (32 * 30) - 192, UIRectType::GAME, 0, 0, NULL, 0 };
	gs->ui_system.rects[1] = { 0, 0, 0, (32 * 40), 192, UIRectType::BOX, 3, 0, NULL, 0 };
	gs->ui_system.rects[2] = { 0, 0, 192, (32 * 40) - (32 * 32), (32 * 30) - 192, UIRectType::BOX, 3, 0, NULL, 0 };
	gs->ui_system.rects[3] = { 1, 500, 500, 128, 192, UIRectType::BOX, 3, 0, NULL, 0 };

	char* str1 = (char*)"Button 1";
	char* str2 = (char*)"Button 2";
	char* str3 = (char*)"Button 3";
	gs->ui_system.rects[4] = { 2, 509, 509, 110, 22, UIRectType::BUTTON, 1, (int)strlen(str1), str1, 0 };
	gs->ui_system.rects[5] = { 2, 509, 531, 110, 22, UIRectType::BUTTON, 1, (int)strlen(str2), str2, 0 };
	gs->ui_system.rects[6] = { 2, 509, 553, 110, 22, UIRectType::BUTTON, 1, (int)strlen(str3), str3, 0 };

	gs->ui_system.rects[7] = { 1, 9, 9, 38, 38, UIRectType::IMAGE, 1, 0, NULL, 0 };

	const int tilemap_width = 200;
	const int tilemap_height = 100;
	const int num_units = 3;

	gs->tilemap = { tilemap_width, tilemap_height, NULL, num_units, NULL };
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

			if (x == 100 && y == 99) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::WATER;
			}
		}
	}

	uint32_t id_counter = 0;
	gs->tilemap.units = (Unit*)malloc(sizeof(Unit) * num_units);
	gs->tilemap.units[0].type = UnitType::ARMY;
	gs->tilemap.units[0].id = id_counter++;
	gs->tilemap.units[0].pos_x = 3;
	gs->tilemap.units[0].pos_y = 3;
	gs->tilemap.units[0].max_hp = 10;
	gs->tilemap.units[0].current_hp = 10;
	gs->tilemap.units[0].attack = 3;

	gs->tilemap.units[1].type = UnitType::ARMY;
	gs->tilemap.units[1].id = id_counter++;
	gs->tilemap.units[1].pos_x = 3;
	gs->tilemap.units[1].pos_y = 4;
	gs->tilemap.units[1].max_hp = 10;
	gs->tilemap.units[1].current_hp = 10;
	gs->tilemap.units[1].attack = 3;

	gs->tilemap.units[2].type = UnitType::NAVY;
	gs->tilemap.units[2].id = id_counter++;
	gs->tilemap.units[2].pos_x = 13;
	gs->tilemap.units[2].pos_y = 15;
	gs->tilemap.units[2].max_hp = 10;
	gs->tilemap.units[2].current_hp = 10;
	gs->tilemap.units[2].attack = 3;
}

void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt) {
	BeginTimer(CT_GAME_UPDATE);
	GameState* gs = (GameState*)gameMemory->data;

	// ========================================================================
	// Camera Update

	KeyboardState key = gameInput->keyboard;
	MouseState mouse = gameInput->mouse;

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
	if (gs->x >= (200.0f * 32.0f * gs->s - (32.0f * 32.0f))) gs->x = (200.0f * 32.0f * gs->s - (32.0f * 32.0f));
	if (gs->y >= (100.0f * 32.0f * gs->s - (32.0f * 24.0f))) gs->y = (100.0f * 32.0f * gs->s - (32.0f * 24.0f));

	// end Camera Update
	// ========================================================================

	if (keyReleased(mouse.left)) {
		int32_t ui_rect_clicked = UIClick(&gs->ui_system, mouse.x, mouse.y);
		
		char buffer[256];
		snprintf(buffer, 256, "Mouse Left Release, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
		snprintf(buffer, 256, "UI Rect Clicked: %d\n", ui_rect_clicked);
		DebugPrint(buffer);
		
		if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::GAME) {
			UIRect r = gs->ui_system.rects[ui_rect_clicked];	
			int32_t tile_x = 0, tile_y = 0;
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, gs->x, gs->y, 32, 32, &tile_x, &tile_y);
			
			// get id of unit in clicked tile if any
			int32_t clicked_unit = -1;
			for (int i = 0; i < gs->tilemap.num_units; i++) {
				int32_t x = gs->tilemap.units[i].pos_x;
				int32_t y = gs->tilemap.units[i].pos_y;
				if (tile_x == x && tile_y == y) {
					clicked_unit = i;
				}
			}

			if (gs->selected_unit == -1) {
				// case : have no unit selected
				gs->selected_unit = clicked_unit;
			}
			else if (0 <= gs->selected_unit && gs->selected_unit < gs->tilemap.num_units) {
				// case : have a unit selected
				Unit* unit = &(gs->tilemap.units[gs->selected_unit]);
				int32_t diff_x = labs(tile_x - unit->pos_x);
				int32_t diff_y = labs(tile_y - unit->pos_y);
				if ((diff_x == 1 && (diff_y == 0 || diff_y == 1)) || (diff_y == 1 && (diff_x == 0 || diff_x == 1))) {
					// clicked tile is adjacent to selected unit's position
					
					// so we move the unit to the selected position
					gs->tilemap.units[gs->selected_unit].pos_x = tile_x;
					gs->tilemap.units[gs->selected_unit].pos_y = tile_y;
				}
				else if ((clicked_unit != gs->selected_unit) && (clicked_unit != -1)) {
					// clicked tile is not adjacent, and we clicked a different unit

					// so we select that unit instead
					gs->selected_unit = clicked_unit;
				}
			}


			snprintf(buffer, 256, "Tile Clicked: %d, %d\nUnit Selected: %d, Unit Clicked: %d\n", tile_x, tile_y, gs->selected_unit, clicked_unit);
			DebugPrint(buffer);
		}
		else if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::BUTTON) {
			snprintf(buffer, 256, "Button %d Pressed\n", ui_rect_clicked - 3);
			DebugPrint(buffer);
		}
	}
	if (keyPressed(mouse.left)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse Left Down, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyReleased(mouse.middle)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse Middle Release, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyPressed(mouse.middle)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse Middle Down, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyReleased(mouse.right)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse Right Released, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyPressed(mouse.right)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse Right Down, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyReleased(mouse.x1)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse X1 Released, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyPressed(mouse.x1)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse X1 Down, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyReleased(mouse.x2)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse X2 Released, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}
	if (keyPressed(mouse.x2)) {
		char buffer[256];
		snprintf(buffer, 256, "Mouse X2 Down, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
	}

	EndTimer(CT_GAME_UPDATE);
}

// End Game Core
// ============================================================================

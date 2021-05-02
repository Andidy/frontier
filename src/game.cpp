#include "game.h"

extern CycleCounter global_cycle_counter;

// ============================================================================
// UI System

UIRect CreateUIRect(int layer, int32_t pos_x, int32_t pos_y, int32_t width, int32_t height, bool visible, int line_width) {
	UIRect result;
	result.layer = layer;
	result.x = pos_x;
	result.y = pos_y;
	result.w = width;
	result.h = height;
	result.visible = visible;
	result.type = UIRectType::BOX;
	result.line_width = line_width;
	result.text = NULL;
	result.text_len = 0;
	result.bitmap_index = 0;
	return result;
}

UIRect CreateUIText(int layer, int32_t pos_x, int32_t pos_y, int32_t width, int32_t height, bool visible) {
	UIRect result;
	result.layer = layer;
	result.x = pos_x;
	result.y = pos_y;
	result.w = width;
	result.h = height;
	result.visible = visible;
	result.type = UIRectType::TEXT;
	result.line_width = 0;
	result.text = NULL;
	result.text_len = 0;
	result.bitmap_index = 0;
	return result;
}

UIRect CreateUIImage(int layer, int32_t pos_x, int32_t pos_y, int32_t width, int32_t height, bool visible, int line_width, int bitmap_index) {
	UIRect result;
	result.layer = layer;
	result.x = pos_x;
	result.y = pos_y;
	result.w = width;
	result.h = height;
	result.visible = visible;
	result.type = UIRectType::IMAGE;
	result.line_width = line_width;
	result.text = NULL;
	result.text_len = 0;
	result.bitmap_index = bitmap_index;
	return result;
}

UIRect CreateUIButton(int layer, int32_t pos_x, int32_t pos_y, int32_t width, int32_t height, bool visible, int line_width, char* text) {
	UIRect result;
	result.layer = layer;
	result.x = pos_x;
	result.y = pos_y;
	result.w = width;
	result.h = height;
	result.visible = visible;
	result.type = UIRectType::BUTTON;
	result.line_width = line_width;
	result.text = text;
	result.text_len = (int32_t)strlen(text);
	result.bitmap_index = 0;
	return result;
}

UIRect CreateUITilemap(int layer, int32_t pos_x, int32_t pos_y, int32_t width, int32_t height, bool visible) {
	UIRect result;
	result.layer = layer;
	result.x = pos_x;
	result.y = pos_y;
	result.w = width;
	result.h = height;
	result.visible = visible;
	result.type = UIRectType::TILEMAP;
	result.line_width = 0;
	result.text = NULL;
	result.text_len = 0;
	result.bitmap_index = 0;
	return result;
}

UIRect CreateUIGame(int layer, int32_t pos_x, int32_t pos_y, int32_t width, int32_t height, bool visible) {
	UIRect result;
	result.layer = layer;
	result.x = pos_x;
	result.y = pos_y;
	result.w = width;
	result.h = height;
	result.visible = visible;
	result.type = UIRectType::GAME;
	result.line_width = 0;
	result.text = NULL;
	result.text_len = 0;
	result.bitmap_index = 0;
	return result;
}

int32_t UIClick(UISystem* ui_system, int32_t x, int32_t y) {
	
	bool hits[ui_system->NUM_RECTS];

	for (int i = 0; i < ui_system->NUM_RECTS; i++) {
		hits[i] = false;
		if (ui_system->rects[i].x < x && x < (ui_system->rects[i].x + ui_system->rects[i].w) &&
			ui_system->rects[i].y < y && y < (ui_system->rects[i].y + ui_system->rects[i].h)) {
			hits[i] = ui_system->rects[i].visible;
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

	gs->tilemap = { 0, 0, NULL, 0, NULL };

	if (LoadGameSettings(gs)) {
		DebugPrint((char*)"Successfully Loaded Game Settings Json\n");
	}

	gs->game_tick = 0;
	gs->tick_rate = 1;
	gs->tick_timer = 0.0f;

	gs->x = 0;
	gs->y = 0;
	gs->s = 1;
	gs->selected_unit = -1;

	// ui game rect
	gs->ui_system.rects[0] = CreateUIGame(0, 256, 192, (32 * 40) - 256, (32 * 30) - 192, true);
	
	// ui box test rects
	gs->ui_system.rects[1] = CreateUIRect(0, 0, 0, (32 * 40), 192, true, 3);
	gs->ui_system.rects[2] = CreateUIRect(0, 0, 192, (32 * 40) - (32 * 32), (32 * 30) - 192, true, 3);
	
	// ui button test rects
	gs->ui_system.rects[3] = CreateUIRect(1, 500, 500, 128, 192, true, 3);
	char* str1 = (char*)"Button 1";
	char* str2 = (char*)"Button 2";
	char* str3 = (char*)"Button 3";
	gs->ui_system.rects[4] = CreateUIButton(2, 509, 509, 110, 22, true, 1, str1);
	gs->ui_system.rects[5] = CreateUIButton(2, 509, 531, 110, 22, true, 1, str2);
	gs->ui_system.rects[6] = CreateUIButton(2, 509, 553, 110, 22, true, 1, str3);

	// UIImage test rect
	gs->ui_system.rects[7] = CreateUIImage(1, 9, 401, 38, 38, true, 1, 0);

	// unit menu ui elements
	gs->ui_system.rects[8] = CreateUIRect(1, 0, 0, 1, 1, false, 1);
	gs->ui_system.rects[9] = CreateUIText(2, 0, 0, 0, 0, false);
	
	// debug game tick display
	gs->ui_system.rects[10] = CreateUIText(2, 9, 9, 100, 100, true);

	// Tilemap for map editing
	gs->ui_system.rects[11] = CreateUITilemap(1, 9, 192+9, 64, 64, true);

	int tilemap_width = gs->tilemap.width;
	int tilemap_height = gs->tilemap.height;
	int num_units = gs->tilemap.num_units;

	gs->tilemap.tiles = (Tile*)calloc(tilemap_width * tilemap_height, sizeof(Tile));
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
			if (x == 199 && y == 99) {
				gs->tilemap.tiles[x + tilemap_width * y].type = TileType::MOUNTAIN;
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

	gs->etm_tile_x = -1;
	gs->etm_tile_y = -1;

	gs->editing_tilemap = { 2, 2, NULL, 0, NULL };
	gs->editing_tilemap.tiles = (Tile*)calloc(2 * 2, sizeof(Tile));
	for (int i = 0; i < 4; i++) {
		gs->editing_tilemap.tiles[i].type = (TileType)i;
	}
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
	// Game World Ticks
	gs->tick_timer += dt;
	if (gs->tick_timer > (1000.0f / (f32)gs->tick_rate)) {
		gs->tick_timer = 0.0f;
		gs->game_tick += 1;
	}

	if (keyReleased(key.one)) {
		gs->tick_rate = 1;
	}
	else if (keyReleased(key.two)) {
		gs->tick_rate = 2;
	}
	else if (keyReleased(key.three)) {
		gs->tick_rate = 4;
	}
	else if (keyReleased(key.four)) {
		gs->tick_rate = 8;
	}
	else if (keyReleased(key.five)) {
		gs->tick_rate = 16;
	}

	{
		int len = snprintf(gs->debug_text_buffer, 256, "Game Tick: %lld, Tick Rate: %d\nLast Unit Selected: %d\n", gs->game_tick, gs->tick_rate, gs->selected_unit);
		gs->ui_system.rects[10].text = gs->debug_text_buffer;
		gs->ui_system.rects[10].text_len = len;
	}

	// end Game World Ticks
	// ========================================================================
	// Mouse Input Controls

	if (keyReleased(mouse.left)) {
		int32_t ui_rect_clicked = UIClick(&gs->ui_system, mouse.x, mouse.y);
		
		char buffer[256];
		snprintf(buffer, 256, "Mouse Left Release, Mouse Position: %d, %d\n", mouse.x, mouse.y);
		DebugPrint(buffer);
		snprintf(buffer, 256, "UI Rect Clicked: %d\n", ui_rect_clicked);
		DebugPrint(buffer);
		
		if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::GAME) {
			// Hide the context menus because we clicked outside of them
			gs->ui_system.rects[8].visible = false;
			gs->ui_system.rects[9].visible = false;

			// Handle the click in the game window
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
				
				if (((diff_x == 1 && (diff_y == 0 || diff_y == 1)) || (diff_y == 1 && (diff_x == 0 || diff_x == 1))) && (clicked_unit == -1)) {
					// clicked tile is adjacent to selected unit's position, and no unit is in the adjacent tile
					
					// so we move the unit to the selected position
					gs->tilemap.units[gs->selected_unit].pos_x = tile_x;
					gs->tilemap.units[gs->selected_unit].pos_y = tile_y;
				}
				else if (((diff_x == 1 && (diff_y == 0 || diff_y == 1)) || (diff_y == 1 && (diff_x == 0 || diff_x == 1))) && (0 <= clicked_unit && clicked_unit < gs->tilemap.num_units)) {
					// clicked an adjacent tile with a unit in it
					
					// so attack the unit
					gs->tilemap.units[clicked_unit].current_hp -= gs->tilemap.units[gs->selected_unit].attack;
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
		else if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::TILEMAP) {
			// Handle the click in the tilemap
			UIRect r = gs->ui_system.rects[ui_rect_clicked];
			int32_t tile_x = 0, tile_y = 0;
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, 0, 0, 32, 32, &tile_x, &tile_y);
			
			gs->etm_tile_x = tile_x;
			gs->etm_tile_y = tile_y;

			snprintf(buffer, 256, "Tile: %d, %d\n", tile_x, tile_y);
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
		int32_t ui_rect_clicked = UIClick(&gs->ui_system, mouse.x, mouse.y);

		char buffer[256];
		snprintf(buffer, 256, "Mouse Right Release, Mouse Position: %d, %d\n", mouse.x, mouse.y);
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

			if (0 <= clicked_unit && clicked_unit < gs->tilemap.num_units) {
				// if we clicked on a unit, populate and open the unit menu
				Unit* unit = &(gs->tilemap.units[clicked_unit]);
				
				int len = snprintf(gs->unit_info_buffer, 256, "ID: %d, POS: %d, %d\nHP: %d/%d\nAttack: %d", unit->id, unit->pos_x, unit->pos_y, unit->current_hp, unit->max_hp, unit->attack);
				
				int line_width = 3;
				gs->ui_system.rects[8].visible = true;
				gs->ui_system.rects[8].x = mouse.x;
				gs->ui_system.rects[8].y = mouse.y;
				gs->ui_system.rects[8].line_width = 3;
				gs->ui_system.rects[8].w = 22 * 9;
				gs->ui_system.rects[8].h = 3 * 16 + 2 * 3 * line_width;

				gs->ui_system.rects[9].visible = true;
				gs->ui_system.rects[9].x = mouse.x + 3 * line_width;
				gs->ui_system.rects[9].y = mouse.y + 3 * line_width;
				gs->ui_system.rects[9].text = gs->unit_info_buffer;
				gs->ui_system.rects[9].text_len = len;
			}
			else if (gs->etm_tile_x != -1 && gs->etm_tile_y != -1) {
				// we didn't click a unit, so we are editing the map
				gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].type = (TileType)(gs->etm_tile_x + gs->editing_tilemap.width * gs->etm_tile_y);
			}
		}
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

	// end Mouse Input Controls
	// ============================================================================

	EndTimer(CT_GAME_UPDATE);
}

// End Game Core
// ============================================================================

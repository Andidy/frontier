#include "game.h"

extern CycleCounter global_cycle_counter;
extern Bitmap blue_noise_tex;

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
// Tilemap

/*
	Check if a two tiles are neighbors in the given tilemap
	Params:
	x,y: the primary tile position
	nx, ny: the potential neighbor tile position 
*/
bool ValidNeighbor(Tilemap* tm, int32_t x, int32_t y, int32_t nx, int32_t ny) {
	int width = tm->width;
	int height = tm->height;

	if (!(0 <= x && x < width)) {
		return false;
	}
	if (!(0 <= y && y < height)) {
		return false;
	}
	if (!(0 <= nx && nx < width)) {
		return false;
	}
	if (!(0 <= ny && ny < height)) {
		return false;
	}

	return (labs(x - nx) == 1) || (labs(y-ny) == 1);
}



// end Tilemap
// ============================================================================
// Game Core

void InitGameState(Memory* gameMemory) {
	GameState* gs = (GameState*)gameMemory->data;

	gs->tilemap = { 0, 0, NULL, 0, NULL };

	if (LoadGameSettings(gs)) {
		DebugPrint((char*)"Successfully Loaded Game Settings Json\n");
	}
	else {
		DebugPrint((char*)"YO DUMMY SOMETHING BROKE\n\n\n\n\n\n\nVERY BROKEY\n");
	}

	gs->frame_ticked = false;
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
	gs->ui_system.rects[3] = CreateUIRect(1, 500, 500, 128, 192, false, 3);
	char* str1 = (char*)"Button 1";
	char* str2 = (char*)"Button 2";
	char* str3 = (char*)"Button 3";
	gs->ui_system.rects[4] = CreateUIButton(2, 509, 509, 110, 22, false, 1, str1);
	gs->ui_system.rects[5] = CreateUIButton(2, 509, 531, 110, 22, false, 1, str2);
	gs->ui_system.rects[6] = CreateUIButton(2, 509, 553, 110, 22, false, 1, str3);

	// UIImage test rect
	gs->ui_system.rects[7] = CreateUIImage(1, 9, 401, 38, 38, true, 1, 0);

	// unit menu ui elements
	gs->ui_system.rects[8] = CreateUIRect(1, 0, 0, 1, 1, false, 1);
	gs->ui_system.rects[9] = CreateUIText(2, 0, 0, 0, 0, false);
	
	// debug game tick display
	gs->ui_system.rects[10] = CreateUIText(2, 9, 9, 100, 100, true);

	// Tilemap for map editing
	gs->ui_system.rects[11] = CreateUITilemap(1, 9, 192+22+9, 64, 64, true);

	// buttons for switching between tile terrain, feature, and structure
	gs->ui_system.rects[12] = CreateUIButton(1, 9, 192 + 9, 15, 22, true, 1, (char*)"<");
	gs->ui_system.rects[13] = CreateUIButton(1, 9+75, 192 + 9, 15, 22, true, 1, (char*)">");

	// text for showing whether we have terrain, feature, or structure
	gs->ui_system.rects[14] = CreateUIText(1, 9 + 15, 192 + 9, 60, 22, true);

	// buttons for switching between tile types within the categories
	gs->ui_system.rects[15] = CreateUIButton(1, 9, 192 + 31, 15, 22, true, 1, (char*)"<");
	gs->ui_system.rects[16] = CreateUIButton(1, 9 + 75, 192 + 31, 15, 22, true, 1, (char*)">");

	// text for showing tile type index
	gs->ui_system.rects[17] = CreateUIText(1, 9 + 15, 192 + 31, 60, 22, true);

	// resource counters
	strncpy_s(gs->resource_names_buffer[(int)Resource::NONE], 16, (char*)"NONE", _TRUNCATE);
	strncpy_s(gs->resource_names_buffer[(int)Resource::WHEAT], 16, (char*)"Wheat", _TRUNCATE);
	strncpy_s(gs->resource_names_buffer[(int)Resource::APPLES], 16, (char*)"Apples", _TRUNCATE);
	strncpy_s(gs->resource_names_buffer[(int)Resource::MONEY], 16, (char*)"Money", _TRUNCATE);

	gs->ui_system.rects[18] = CreateUIText(1, 9, 53, 100, 16, true);
	gs->ui_system.rects[19] = CreateUIText(1, 9, 53+16, 100, 16, true);
	gs->ui_system.rects[20] = CreateUIText(1, 9, 53+32, 100, 16, true);
	gs->ui_system.rects[21] = CreateUIText(1, 9, 53+48, 100, 16, true);

	// testing colored text and colored text with background
	gs->ui_system.rects[22] = CreateUIText(1, 400, 9, 100, 16, true);
	gs->ui_system.rects[22].text_color = {255, 125, 50, 255};
	gs->ui_system.rects[22].type = UIRectType::COLORED_TEXT;
	char* test1 = (char*)"testing colored text";
	gs->ui_system.rects[22].text = test1;
	gs->ui_system.rects[22].text_len = strlen(test1);
	gs->ui_system.rects[23] = CreateUIText(1, 400, 25, 100, 16, true);
	gs->ui_system.rects[23].text_color = { 255, 255, 50, 255 };
	gs->ui_system.rects[23].background_color = { 50, 50, 50, 255 };
	gs->ui_system.rects[23].type = UIRectType::COLORED_TEXT_BACKGROUND;
	char* test2 = (char*)"testing colored text with a background color";
	gs->ui_system.rects[23].text = test2;
	gs->ui_system.rects[23].text_len = strlen(test2);

	int tilemap_width = gs->tilemap.width;
	int tilemap_height = gs->tilemap.height;
	int num_units = gs->tilemap.num_units;

	gs->tilemap.tiles = (Tile*)calloc(tilemap_width * tilemap_height, sizeof(Tile));
	for (int y = 0; y < tilemap_height; y++) {
		for (int x = 0; x < tilemap_width; x++) {
			gs->tilemap.tiles[x + tilemap_width * y].terrain = TileTerrain::NONE;

			gs->tilemap.tiles[x + tilemap_width * y].terrain_subtiles[0] = 0;
			gs->tilemap.tiles[x + tilemap_width * y].terrain_subtiles[1] = 0;
			gs->tilemap.tiles[x + tilemap_width * y].terrain_subtiles[2] = 0;
			gs->tilemap.tiles[x + tilemap_width * y].terrain_subtiles[3] = 0;
			
			gs->tilemap.tiles[x + tilemap_width * y].terrain_variants[0] = 0;
			gs->tilemap.tiles[x + tilemap_width * y].terrain_variants[1] = 0;
			gs->tilemap.tiles[x + tilemap_width * y].terrain_variants[2] = 0;
			gs->tilemap.tiles[x + tilemap_width * y].terrain_variants[3] = 0;

			gs->tilemap.tiles[x + tilemap_width * y].terrain_variant_fixed = true;

			gs->tilemap.tiles[x + tilemap_width * y].feature = TileFeature::NONE;
			gs->tilemap.tiles[x + tilemap_width * y].structure = TileStructure::NONE;
		}
	}

	uint32_t id_counter = 0;
	gs->tilemap.units = (Unit*)malloc(sizeof(Unit) * num_units);
	gs->tilemap.units[0].type = UnitType::ARMY;
	gs->tilemap.units[0].id = id_counter++;
	gs->tilemap.units[0].pos_x = 10;
	gs->tilemap.units[0].pos_y = 3;
	gs->tilemap.units[0].max_hp = 10;
	gs->tilemap.units[0].current_hp = 10;
	gs->tilemap.units[0].attack = 3;

	gs->tilemap.units[1].type = UnitType::ARMY;
	gs->tilemap.units[1].id = id_counter++;
	gs->tilemap.units[1].pos_x = 11;
	gs->tilemap.units[1].pos_y = 4;
	gs->tilemap.units[1].max_hp = 10;
	gs->tilemap.units[1].current_hp = 10;
	gs->tilemap.units[1].attack = 3;

	gs->tilemap.units[2].type = UnitType::NAVY;
	gs->tilemap.units[2].id = id_counter++;
	gs->tilemap.units[2].pos_x = 10;
	gs->tilemap.units[2].pos_y = 15;
	gs->tilemap.units[2].max_hp = 10;
	gs->tilemap.units[2].current_hp = 10;
	gs->tilemap.units[2].attack = 3;

	gs->edit_type = 0;
	_itoa_s(gs->edit_type, gs->edit_type_buffer, 10);
	gs->ui_system.rects[14].text = gs->edit_type_buffer;
	gs->ui_system.rects[14].text_len = (int)strlen(gs->edit_type_buffer);

	gs->edit_index = 0;
	_itoa_s(gs->edit_index, gs->edit_index_buffer, 10);
	gs->ui_system.rects[17].text = gs->edit_index_buffer;
	gs->ui_system.rects[17].text_len = (int)strlen(gs->edit_index_buffer);

	gs->resources[0] = 0;
	snprintf(gs->resource_counts_buffer[0], 256, "%s: %d", gs->resource_names_buffer[0], gs->resources[0]);
	gs->ui_system.rects[18].text = gs->resource_counts_buffer[0];
	gs->ui_system.rects[18].text_len = (int)strlen(gs->resource_counts_buffer[0]);

	gs->resources[1] = 0;
	snprintf(gs->resource_counts_buffer[1], 256, "%s: %d", gs->resource_names_buffer[1], gs->resources[1]);
	gs->ui_system.rects[19].text = gs->resource_counts_buffer[1];
	gs->ui_system.rects[19].text_len = (int)strlen(gs->resource_counts_buffer[1]);
	
	gs->resources[2] = 0;
	snprintf(gs->resource_counts_buffer[2], 256, "%s: %d", gs->resource_names_buffer[2], gs->resources[2]);
	gs->ui_system.rects[20].text = gs->resource_counts_buffer[2];
	gs->ui_system.rects[20].text_len = (int)strlen(gs->resource_counts_buffer[2]);

	gs->resources[3] = 0;
	snprintf(gs->resource_counts_buffer[3], 256, "%s: %d", gs->resource_names_buffer[3], gs->resources[3]);
	gs->ui_system.rects[21].text = gs->resource_counts_buffer[3];
	gs->ui_system.rects[21].text_len = (int)strlen(gs->resource_counts_buffer[3]);
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
		gs->frame_ticked = true;
	}
	else {
		gs->frame_ticked = false;
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
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, (int)gs->x, (int)gs->y, 32, 32, &tile_x, &tile_y);
			
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
			
			if (ui_rect_clicked == 12) {
				// left arrow for editing tilemap
				gs->edit_type -= 1;
				if (gs->edit_type < 0) gs->edit_type = 0;
			}
			else if (ui_rect_clicked == 13) {
				// right arrow for editing tilemap
				gs->edit_type += 1;
				if (gs->edit_type > 2) gs->edit_type = 2;
			}
			
			if (ui_rect_clicked == 12 || ui_rect_clicked == 13) {
				switch (gs->edit_type) {
				case 0: if (gs->edit_index >= (int)TileTerrain::NUM_TYPES) gs->edit_index = (int)TileTerrain::NUM_TYPES - 1; break;
				case 1: if (gs->edit_index >= (int)TileFeature::NUM_TYPES) gs->edit_index = (int)TileFeature::NUM_TYPES - 1; break;
				case 2: if (gs->edit_index >= (int)TileStructure::NUM_TYPES) gs->edit_index = (int)TileStructure::NUM_TYPES - 1; break;
				default: break;
				}

				_itoa_s(gs->edit_type, gs->edit_type_buffer, 10);
				gs->ui_system.rects[14].text = gs->edit_type_buffer;
				gs->ui_system.rects[14].text_len = (int)strlen(gs->edit_type_buffer);

				_itoa_s(gs->edit_index, gs->edit_index_buffer, 10);
				gs->ui_system.rects[17].text = gs->edit_index_buffer;
				gs->ui_system.rects[17].text_len = (int)strlen(gs->edit_index_buffer);
			}
			
			if (ui_rect_clicked == 15) {
				// left arrow for editing tilemap
				gs->edit_index -= 1;
				if (gs->edit_index < 0) gs->edit_index = 0;
			}
			else if (ui_rect_clicked == 16) {
				// right arrow for editing tilemap
				gs->edit_index += 1;
				switch (gs->edit_type) {
				case 0: if (gs->edit_index >= (int)TileTerrain::NUM_TYPES) gs->edit_index = (int)TileTerrain::NUM_TYPES - 1; break;
				case 1: if (gs->edit_index >= (int)TileFeature::NUM_TYPES) gs->edit_index = (int)TileFeature::NUM_TYPES - 1; break;
				case 2: if (gs->edit_index >= (int)TileStructure::NUM_TYPES) gs->edit_index = (int)TileStructure::NUM_TYPES - 1; break;
				default: break;
				}
			}

			if (ui_rect_clicked == 15 || ui_rect_clicked == 16) {
				_itoa_s(gs->edit_index, gs->edit_index_buffer, 10);
				gs->ui_system.rects[17].text = gs->edit_index_buffer;
				gs->ui_system.rects[17].text_len = (int)strlen(gs->edit_index_buffer);
			}

			snprintf(buffer, 256, "Button \"%s\" Pressed\n", gs->ui_system.rects[ui_rect_clicked].text);
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
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, (int)gs->x, (int)gs->y, 32, 32, &tile_x, &tile_y);

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
			
			else {
				// we didn't click a unit, so we are editing the map

				switch (gs->edit_type) {
				case 0: {
					gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain = (TileTerrain)gs->edit_index;
					gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain_variant_fixed = false;
				} break;
				case 1: {
					gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].feature = (TileFeature)gs->edit_index;
				} break;
				case 2: {
					gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].structure = (TileStructure)gs->edit_index;
				} break;
				default: break;
				}
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

	if (keyReleased(key.z)) {
		int32_t ui_rect_clicked = UIClick(&gs->ui_system, mouse.x, mouse.y);

		if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::GAME) {
			UIRect r = gs->ui_system.rects[ui_rect_clicked];
			int32_t tile_x = 0, tile_y = 0;
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, (int)gs->x, (int)gs->y, 32, 32, &tile_x, &tile_y);

			gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain = TileTerrain::GRASS;
			gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain_variant_fixed = false;
		}
	}
	if (keyReleased(key.x)) {
		int32_t ui_rect_clicked = UIClick(&gs->ui_system, mouse.x, mouse.y);

		if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::GAME) {
			UIRect r = gs->ui_system.rects[ui_rect_clicked];
			int32_t tile_x = 0, tile_y = 0;
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, (int)gs->x, (int)gs->y, 32, 32, &tile_x, &tile_y);

			gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain = TileTerrain::WATER;
			gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain_variant_fixed = false;
		}
	}
	if (keyReleased(key.c)) {
		int32_t ui_rect_clicked = UIClick(&gs->ui_system, mouse.x, mouse.y);

		if (gs->ui_system.rects[ui_rect_clicked].type == UIRectType::GAME) {
			UIRect r = gs->ui_system.rects[ui_rect_clicked];
			int32_t tile_x = 0, tile_y = 0;
			ScreenToTile(mouse.x, mouse.y, r.x, r.y, (int)gs->x, (int)gs->y, 32, 32, &tile_x, &tile_y);

			gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain = TileTerrain::DEBUG;
			gs->tilemap.tiles[tile_x + gs->tilemap.width * tile_y].terrain_variant_fixed = true;
		}
	}

	// ========================================================================
	// Resource extraction

	if (gs->frame_ticked) {
		for (int i = 0; i < gs->tilemap.width * gs->tilemap.height; i++) {
			Tile tile = gs->tilemap.tiles[i];
			switch (tile.structure) {
				case TileStructure::FARMHOUSE:
				{
					gs->resources[(int)Resource::MONEY] += 1;
				} break;
				case TileStructure::FIELD:
				{
					gs->resources[(int)Resource::WHEAT] += 1;
				} break;
				case TileStructure::ORCHARD:
				{
					gs->resources[(int)Resource::APPLES] += 1;
				} break;
				default: break;
			}
		}

		snprintf(gs->resource_counts_buffer[0], 256, "%s: %d", gs->resource_names_buffer[0], gs->resources[0]);
		gs->ui_system.rects[18].text = gs->resource_counts_buffer[0];
		gs->ui_system.rects[18].text_len = (int)strlen(gs->resource_counts_buffer[0]);

		snprintf(gs->resource_counts_buffer[1], 256, "%s: %d", gs->resource_names_buffer[1], gs->resources[1]);
		gs->ui_system.rects[19].text = gs->resource_counts_buffer[1];
		gs->ui_system.rects[19].text_len = (int)strlen(gs->resource_counts_buffer[1]);

		snprintf(gs->resource_counts_buffer[2], 256, "%s: %d", gs->resource_names_buffer[2], gs->resources[2]);
		gs->ui_system.rects[20].text = gs->resource_counts_buffer[2];
		gs->ui_system.rects[20].text_len = (int)strlen(gs->resource_counts_buffer[2]);

		snprintf(gs->resource_counts_buffer[3], 256, "%s: %d", gs->resource_names_buffer[3], gs->resources[3]);
		gs->ui_system.rects[21].text = gs->resource_counts_buffer[3];
		gs->ui_system.rects[21].text_len = (int)strlen(gs->resource_counts_buffer[3]);
	}
	
	// end Resource extraction
	// ========================================================================

	EndTimer(CT_GAME_UPDATE);
}

// End Game Core
// ============================================================================

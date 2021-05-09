#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "universal.h"

// ============================================================================
// Tilemap

enum class UnitType {
	ARMY = 1,
	NAVY = 2,

	NUM_TYPES
};

struct Unit {
	UnitType type;

	uint32_t id;

	int32_t pos_x;
	int32_t pos_y;

	int32_t max_hp = 10;
	int32_t current_hp = 10;
	int32_t attack = 3;
};

enum class TileTerrain {
	NONE = 0,
	DEBUG = 1,
	GRASS = 2,
	GRASS_CLIFF = 3,
	WATER = 4,
	DESERT = 5,
	SNOW = 6,

	NUM_TYPES
};

enum class TileFeature {
	NONE = 0,
	MOUNTAIN = 1,
	HILLS = 2,
	FOREST = 3,
	WOODS = 4,
	SWAMP = 5,
	
	NUM_TYPES
};

enum class TileStructure {
	NONE = 0,
	HOUSE = 1,
	FORT = 2,
	MINE = 3,
	RAIL = 4,
	WALLS = 5,
	
	NUM_TYPES
};

struct Tile {
	TileTerrain terrain;
	TileFeature feature;
	TileStructure structure;

	//	Subtile Layout:
	//	0, 1,
	//	2, 3	
	int terrain_subtiles[4];
	int terrain_variants[4];
	bool terrain_variant_fixed;

	int feature_subtiles[4];
	int feature_variants[4];
	bool feature_variant_fixed;

	int structure_subtiles[4];
	int structure_variants[4];
	bool structure_variant_fixed;
};

struct Tilemap {
	bool wrap_horz;
	bool wrap_vert;
	int32_t width;
	int32_t height;
	Tile* tiles;

	int32_t num_units;
	Unit* units;
};

// End Tilemap
// ============================================================================
// UI System

enum class UIRectType {
	BOX,
	TEXT,
	IMAGE,
	BUTTON,
	TILEMAP,
	GAME,
	NUM_RECT_TYPES
};

struct UIRect {
	int32_t layer;
	int32_t x, y;
	int32_t w, h;

	bool visible;

	UIRectType type;
	
	int32_t line_width;
	
	int32_t text_len;
	char* text;

	int32_t bitmap_index;
};

struct UISystem {
	static const int32_t NUM_RECTS = 15;
	UIRect rects[NUM_RECTS];
};

// End UI System
// ============================================================================

struct GameState {
	int64_t game_tick;
	int32_t tick_rate;
	f32 tick_timer;
	char debug_text_buffer[256];

	Tilemap tilemap;

	/*
	Tilemap editing_tilemap;
	TileTerrain etm_tile_type;
	int32_t etm_page;
	char etm_page_buffer[8];
	*/

	int32_t selected_unit;
	char unit_info_buffer[256];

	f32 x;
	f32 y;
	int32_t s;

	int32_t window_width;
	int32_t window_height;

	UISystem ui_system;
};

#endif

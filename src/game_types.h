#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "universal.h"

enum class Direction {
	NONE = 0,
	NORTH = 1,
	EAST = 2,
	SOUTH = 3,
	WEST = 4,
	NUM_TYPES
};

// Expanding Queue ============================================================
// developed based on wikipedia articles on unrolled_linked_lists and queues

struct ExpandingQueueNode {
	ExpandingQueueNode* next;
	static const int num_slots = 4;
	Direction slot[num_slots];
};

struct ExpandingQueue {
	ExpandingQueueNode* front;
	int front_offset;

	ExpandingQueueNode* back;
	int back_offset;

	Pool* pool;

	ExpandingQueue(Pool* pool) {
		this->pool = pool;
		front = (ExpandingQueueNode*)pool->Allocate();
		front_offset = 0;
		back = front;
		back_offset = 0;
	}

	bool IsEmpty() {
		return ((front == back) && (front_offset == back_offset));
	}

	void Enqueue(Direction item) {
		back->slot[back_offset] = item;
		back_offset += 1;
		if (back_offset == back->num_slots) {
			// allocate a new queue node from pool allocator
			ExpandingQueueNode* temp = (ExpandingQueueNode*)pool->Allocate();

			back_offset = 0;
			back->next = temp;
			back = temp;
		}
	}

	Direction Dequeue() {
		if (IsEmpty()) {
			return Direction::NONE;
		}
		Direction temp = front->slot[front_offset];
		front_offset += 1;
		if (front_offset == front->num_slots) {
			front_offset = 0;

			// free the queue node front points to;
			ExpandingQueueNode* temp_ptr = front->next;
			pool->Free(front);
			front = temp_ptr;
		}
		return temp;
	}
};

// ============================================================================
// Characters

struct Character {
	uint32_t id;


};

// end Characters
// ============================================================================
// Gameplay Resources

enum class Resource {
	NONE = 0,
	WHEAT = 1,
	APPLES = 2,
	MONEY = 3,
	WOOD = 4,
	FLOUR = 5,
	BREAD = 6,
	CIDER = 7,
	PLANKS = 8,
	STONE = 9,

	NUM_TYPES
};

// end Gameplay Resources
// ============================================================================
// Gameplay Unit

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

	ExpandingQueue path;
};

// end Gameplay Unit
// ============================================================================
// Tilemap

enum class TileTerrain {
	NONE = 0,
	DEBUG = 1,
	GRASS = 2,
	GRASS_CLIFF = 3,
	WATER = 4,
	DESERT = 5,

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

enum class TileStructureType {
	NONE = 0,
	FARMHOUSE = 1,
	FIELD = 2,
	ORCHARD = 3,

	WOODCUTTER = 4,
	WINDMILL = 5,
	SAWMILL = 6,
	BAKERY = 7,
	BREWERY = 8,
	QUARRY = 9,

	TEST,
	NUM_TYPES
};

struct ResourceAmount {
	Resource resource;
	int amount;
};

struct BuildingTemplate {
	int uid;
	TileStructureType type;
	
	int num_inputs;
	int num_outputs;
	int production_time;
	ResourceAmount* production_input;
	ResourceAmount* production_output;

	int num_build_resources;
	Resource* resources_to_build;
	int* build_amounts;
	int ticks_to_build;
};

struct Building {
	TileStructureType type;

	int construction_progress;
	int construction_max_time;

	int production_progress;
	int production_max_time;
};

struct Tile {
	TileTerrain terrain;
	TileFeature feature;
	TileStructureType structure;
	Building building;

	//	Subtile Layout:
	//	0, 1,
	//	2, 3	
	int terrain_subtiles[4];
	int terrain_variants[4];
	bool terrain_variant_fixed;

	//int feature_subtiles[4];
	//int feature_variants[4];
	//bool feature_variant_fixed;

	//int structure_subtiles[4];
	//int structure_variants[4];
	//bool structure_variant_fixed;
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
	COLORED_TEXT,
	COLORED_TEXT_BACKGROUND,
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
	Color text_color;
	Color background_color;

	int32_t bitmap_index;
};

struct UISystem {
	static const int32_t NUM_RECTS = 30;
	UIRect rects[NUM_RECTS];
};

// End UI System
// ============================================================================

struct GameState {
	bool frame_ticked;
	int64_t game_tick;
	int32_t tick_rate;
	f32 tick_timer;
	char debug_text_buffer[256];

	bool load_textures;

	Tilemap tilemap;

	int32_t num_building_templates;
	BuildingTemplate building_templates[(int)TileStructureType::NUM_TYPES];

	char edit_type_buffer[8];
	char edit_index_buffer[8];
	int edit_type = 0;
	int edit_index = 0;

	Pool unit_path_pool;
	int32_t selected_unit;
	char unit_info_buffer[256];

	f32 x;
	f32 y;
	int32_t s;

	int32_t window_width;
	int32_t window_height;

	UISystem ui_system;

	char temp_buffer[8];
	char resource_names_buffer[(int)Resource::NUM_TYPES][16];
	char resource_counts_buffer[(int)Resource::NUM_TYPES][32];
	int resources[(int)Resource::NUM_TYPES];
};

#endif

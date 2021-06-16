#ifndef RENDERER_H
#define RENDERER_H

#include "universal.h"

#include "game.h"

bool DrawPixel(Bitmap* bitmap, int32_t x, int32_t y, Color color);
bool DrawRect(Bitmap* bitmap, int32_t x, int32_t y, int32_t w, int32_t h, Color color);
bool DrawSprite(Bitmap* bitmap, int32_t x, int32_t y, Bitmap* sprite);
bool DrawSpriteMagnified(Bitmap* bitmap, int32_t x, int32_t y, int32_t scale, Bitmap* sprite);
void DrawUIRect(Bitmap* viewport, int32_t x, int32_t y, int32_t width, int32_t height, int32_t line_width, Color background_color, Color line_color);
void DrawUIText(Bitmap* viewport, int32_t x, int32_t y, char* text, int32_t text_len, Bitmap* font);
void DrawUIText(Bitmap* viewport, int32_t x, int32_t y, char* text, int32_t text_len, Bitmap* font, Color color);
void DrawUIText(Bitmap* viewport, int32_t x, int32_t y, char* text, int32_t text_len, Bitmap* font, Color text_color, Color background_color);

struct TextureAtlas {
	int32_t num_anim_frames;
	int32_t num_subtile_variants;
	Bitmap* frames;
};

void CorrectSTBILoadMemoryLayout(void* memory, int32_t width, int32_t height);

struct TilemapRenderer {
	// width and height of a single tile / sprite
	int32_t tile_width{ 0 };
	int32_t tile_height{ 0 };
	int32_t tile_scale{ 0 };

	// region of world to draw
	int32_t view_x{ 0 };
	int32_t view_y{ 0 };
	int32_t view_w{ 0 };
	int32_t view_h{ 0 };
	Bitmap view_bitmap{NULL, 0, 0};

	// Animation state
	int32_t animation_frame;
	int32_t animation_max_frames;
	f32 animation_frame_time;
	f32 animation_max_frame_time;

	// Textures for the renderer to use
	Bitmap background_grid;
	Bitmap path_arrows;

	static const int32_t num_terrain_atlases = (int)TileTerrain::NUM_TYPES;
	TextureAtlas terrain_atlases[num_terrain_atlases];
	static const int32_t num_structure_atlases = (int)TileStructureType::NUM_TYPES;
	TextureAtlas structure_atlases[num_structure_atlases];
	static const int32_t num_unit_atlases = (int)UnitType::NUM_TYPES;
	TextureAtlas unit_atlases[num_unit_atlases];

	TilemapRenderer();
	TilemapRenderer(int tile_w, int tile_h, int tile_s, int v_x, int v_y, int v_w, int v_h, int anim_max_frames, f32 anim_frame_time, Bitmap bitmap);
	void ResizeViewport(int width, int height);
	void DrawSprite(int32_t world_x, int32_t world_y, int32_t tex_atlas_x, int32_t tex_atlas_y, Bitmap* texture_atlas);
	void DrawSubTile(int32_t world_x, int32_t world_y, int32_t tex_atlas_x, int32_t tex_atlas_y, Bitmap* texture_atlas);
	void DrawSubTiles(int32_t x, int32_t y, int* subtiles, int* variants, Bitmap* texture_atlas);
	Bitmap* GetTerrainAtlas(TileTerrain type);
	Bitmap* GetStructureAtlas(TileStructureType type);
	Bitmap* GetUnitAtlas(UnitType type);
	void DrawTilemap(Tilemap* tilemap);
	void CacheTileRenderingSubtiles(Tilemap* tm);
};

#endif

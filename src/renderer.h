#ifndef RENDERER_H
#define RENDERER_H

#include "universal.h"

#include "game_types.h"

struct Color {
	uchar b;
	uchar g;
	uchar r;
	uchar a;
};

/*
	Bitmap coordinate system is (0, 0) = top left, (width, height) = bottom left
*/
struct Bitmap {
	uchar* buffer;
	int32_t width;
	int32_t height;
};

void CorrectSTBILoadMemoryLayout(void* memory, int32_t width, int32_t height);

bool DrawPixel(Bitmap* bitmap, int32_t x, int32_t y, Color color);
bool DrawRect(Bitmap* bitmap, int32_t x, int32_t y, int32_t w, int32_t h, Color color);
bool DrawSprite(Bitmap* bitmap, int32_t x, int32_t y, Bitmap* sprite);
bool DrawSpriteMagnified(Bitmap* bitmap, int32_t x, int32_t y, int32_t scale, Bitmap* sprite);

struct TilemapRenderer {
	// width and height of a single tile / sprite
	int32_t tile_width{ 0 };
	int32_t tile_height{ 0 };
	int32_t tile_scale{ 0 };

	// number of tiles in x/y axis
	int32_t tilemap_width{ 0 };
	int32_t tilemap_height{ 0 };
	
	// derived value of tile_width(height) * tilemap_width(height)
	int32_t world_width{ 0 };
	int32_t world_height{ 0 };

	// region of world to draw
	int32_t view_x{ 0 };
	int32_t view_y{ 0 };
	int32_t view_w{ 0 };
	int32_t view_h{ 0 };
	Bitmap view_bitmap{NULL, 0, 0};

	Bitmap* sprites[4];

	Bitmap texture_atlases[1];

	TilemapRenderer();
	TilemapRenderer(int tile_w, int tile_h, int tile_s, int tilemap_w, int tilemap_h, int v_x, int v_y, int v_w, int v_h, Bitmap bitmap);
	void DrawSprite(int32_t x, int32_t y, Bitmap* sprite);
	void DrawSprite(int32_t world_x, int32_t world_y, int32_t tex_atlas_x, int32_t tex_atlas_y, Bitmap* texture_atlas);
	void DrawTilemap(GameState* gs);
};

#endif

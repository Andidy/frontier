#include "renderer.h"

/*
	Fixes that stbi_load loads in rgba order instead of argb.
*/
void CorrectSTBILoadMemoryLayout(void* memory, int32_t width, int32_t height) {
	uint32_t* buffer = (uint32_t*)memory;
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			uint32_t temp = buffer[w + width * h];

			uchar r = (uchar)temp;
			uchar g = (uchar)(temp >> 8);
			uchar b = (uchar)(temp >> 16);
			uchar a = (uchar)(temp >> 24);

			// super basic transparency (magenta = transparent)
			if (r == 255 && g == 255 && b == 255 && a == 0) {
				a = 0;
			}
			else {
				a = 255;
			}

			buffer[w + width * h] = (b | g << 8 | r << 16 | a << 24);
		}
	}
}

/*
	Writes a pixel that is colored with parameter color to position x, y 
	in the provided bitmap.
	Bitmap coordinates are 0,0 is top left, width,height is bottom right.
	Returns true if the operation succeeded, false if the provided 
	coordinates are outside the boundaries of the bitmap.
*/
bool DrawPixel(Bitmap* bitmap, int32_t x, int32_t y, Color color) {
	if (x < 0 || x > bitmap->width || y < 0 || y > bitmap->height) {
		return false;
	}
	
	uint32_t* buffer = (uint32_t*)bitmap->buffer;
	buffer[x + bitmap->width * y] = (color.b | color.g << 8 | color.r << 16 | color.a << 24);

	return true;
}

/*
	Draws a rectangle with the given color to the position x, y with the
	provided width(w) and height(h) to the given bitmap. w extends right
	from x, h extends down from y, therefore x,y is top left of the rect
	x+w, y+h is bottom right of the rect.
	Bitmap coordinates are 0,0 is top left, width,height is bottom right.
	Returns true if the operation succeeded, false if any section of the 
	rect is outside the boundaries of the bitmap.
*/
bool DrawRect(Bitmap* bitmap, int32_t x, int32_t y, int32_t w, int32_t h, Color color) {
	if (x < 0 || (x + w) > bitmap->width || y < 0 || (y + h) > bitmap->height ||
		w < 0 || h < 0) {
		return false;
	}

	uint32_t* buffer = (uint32_t*)bitmap->buffer;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			buffer[(x+j) + bitmap->width * (y+i)] = (color.b | color.g << 8 | color.r << 16 | color.a << 24);
		}
	}

	return true;
}

/*
	Draws the sprite at the given x,y position into the bitmap.
	Bitmap coordinates are 0,0 is top left, width,height is bottom right.
	Returns true if the operation succeeded, false if any section of the
	rect is outside the boundaries of the bitmap.
*/
bool DrawSprite(Bitmap* bitmap, int32_t x, int32_t y, Bitmap* sprite) {
	if (x < 0 || (x+sprite->width) > bitmap->width || y < 0 || (y+sprite->height) > bitmap->height) {
		return false;
	}

	uint32_t* bitmap_buffer = (uint32_t*)bitmap->buffer;
	uint32_t* sprite_buffer = (uint32_t*)sprite->buffer;
	for (int i = 0; i < sprite->height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			bitmap_buffer[(x+j) + bitmap->width * (y+i)] = sprite_buffer[j + sprite->width * i];
		}
	}

	return true;
}

/*
	Works the same as draw sprite, except that every pixel in the source
	image is scaled by the scale parameter. So with a scale parameter of
	4 a single pixel in the source image becomes a 4x4 rectangle in the
	bitmap.
*/
bool DrawSpriteMagnified(Bitmap* bitmap, int32_t x, int32_t y, int32_t scale, Bitmap* sprite) {
	if (x < 0 || (x + scale * sprite->width) > bitmap->width || y < 0 || (y + scale * sprite->height) > bitmap->height) {
		return false;
	}

	uint32_t* bitmap_buffer = (uint32_t*)bitmap->buffer;
	uint32_t* sprite_buffer = (uint32_t*)sprite->buffer;
	for (int i = 0; i < sprite->height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			uint32_t pixel = sprite_buffer[j + sprite->width * i];
			for (int k = 0; k < scale * scale; k++) {
				bitmap_buffer[(x + k % scale + j * scale) + bitmap->width * (y + k / scale + i * scale)] = pixel;	
			}
		}
	}

	return true;
}

TilemapRenderer::TilemapRenderer() {}

TilemapRenderer::TilemapRenderer(int tile_w, int tile_h, int tile_s, int tilemap_w, int tilemap_h, int v_x, int v_y, int v_w, int v_h, int anim_max_frames, f32 anim_frame_time, Bitmap bitmap) {
	tile_width = tile_w;
	tile_height = tile_h;
	tile_scale = tile_s;
	tilemap_width = tilemap_w;
	tilemap_height = tilemap_h;
	world_width = tile_w * tile_s * tilemap_w;
	world_height = tile_h * tile_s * tilemap_h;
	view_x = v_x;
	view_y = v_y;
	view_w = v_w;
	view_h = v_h;
	view_bitmap.buffer = bitmap.buffer;
	view_bitmap.width = bitmap.width;
	view_bitmap.height = bitmap.height;
	animation_frame = 0;
	animation_max_frames = anim_max_frames;
	animation_frame_time = 0;
	animation_max_frame_time = anim_frame_time;
}

void TilemapRenderer::DrawSprite(int32_t world_x, int32_t world_y, Bitmap* sprite) {
	
	// for starting corner we want the larger value
	int32_t start_x = (world_x >= view_x) ? world_x : view_x;
	int32_t start_y = (world_y >= view_y) ? world_y : view_y;
	// for ending corner we want the smaller value
	int32_t end_x = (world_x + sprite->width * tile_scale < view_x + view_w) ? world_x + sprite->width * tile_scale : view_x + view_w;
	int32_t end_y = (world_y + sprite->height * tile_scale < view_y + view_h) ? world_y + sprite->height * tile_scale : view_y + view_h;

	uint32_t* bitmap_buffer = (uint32_t*)view_bitmap.buffer;
	uint32_t* sprite_buffer = (uint32_t*)sprite->buffer;

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			
			uint32_t pixel = sprite_buffer[((x - world_x) / tile_scale) + sprite->width * ((y - world_y) / tile_scale)];
			
			int32_t viewport_x = (x - view_x);
			int32_t viewport_y = (y - view_y);

			if (viewport_y < view_bitmap.height && viewport_x < view_bitmap.width) {
				bitmap_buffer[viewport_x + view_bitmap.width * viewport_y] = pixel;
			}
		}
	}
}

/*
	Draw a sprite into the viewport.
	Params:
	world_x / world_y: world coordinates of the sprite being drawn
	tex_atlas_x / tex_atlas_y: the index inside the texture atlas of the sprite
	texture_atlas: the texture atlas where the sprite is located
*/
void TilemapRenderer::DrawSprite(int32_t world_x, int32_t world_y, int32_t tex_atlas_x, int32_t tex_atlas_y, Bitmap* texture_atlas) {

	// for starting corner we want the larger value
	int32_t start_x = (world_x >= view_x) ? world_x : view_x;
	int32_t start_y = (world_y >= view_y) ? world_y : view_y;
	// for ending corner we want the smaller value
	int32_t end_x = (world_x + tile_width * tile_scale < view_x + view_w) ? world_x + tile_width * tile_scale : view_x + view_w;
	int32_t end_y = (world_y + tile_width * tile_scale < view_y + view_h) ? world_y + tile_width * tile_scale : view_y + view_h;

	uint32_t* bitmap_buffer = (uint32_t*)view_bitmap.buffer;
	uint32_t* sprite_buffer = (uint32_t*)texture_atlas->buffer;

	int32_t tex_atlas_off_x = tex_atlas_x * tile_width;
	int32_t tex_atlas_off_y = tex_atlas_y * tile_height;

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			int32_t viewport_x = (x - view_x);
			int32_t viewport_y = (y - view_y);

			int32_t pixel_x = ((x - world_x) / tile_scale + tex_atlas_off_x);
			int32_t pixel_y = ((y - world_y) / tile_scale + tex_atlas_off_y);
			uint32_t pixel = sprite_buffer[pixel_x + texture_atlas->width * pixel_y];

			if (viewport_y < view_bitmap.height && viewport_x < view_bitmap.width) {
				bitmap_buffer[viewport_x + view_bitmap.width * viewport_y] = pixel;
			}
		}
	}
}

void TilemapRenderer::DrawTilemap(GameState* gs) {
	int32_t scaled_tile_width = tile_width * tile_scale;
	int32_t scaled_tile_height = tile_height * tile_scale;

	int start_x = view_x / scaled_tile_width;
	int start_y = view_y / scaled_tile_height;
	int end_x = (view_x + view_w) / scaled_tile_width;
	int end_y = (view_y + view_h) / scaled_tile_height;

	for (int y = start_y; y <= end_y; y++) {
		for (int x = start_x; x <= end_x; x++) {
			switch (gs->tilemap.tiles[x + gs->tilemap.width * y].type) {
				case TileType::NONE: break;
				case TileType::GRASS:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 1, 0, &(tex_atlases[0].frames[animation_frame % tex_atlases[0].num_anim_frames]));
				} break;
				case TileType::WATER:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 0, 1, &tex_atlases[0].frames[animation_frame % tex_atlases[0].num_anim_frames]);
				} break;
				case TileType::MOUNTAIN:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 1, 1, &tex_atlases[0].frames[animation_frame % tex_atlases[0].num_anim_frames]);
				} break;
				case TileType::HOUSE:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 0, 0, &tex_atlases[1].frames[animation_frame % tex_atlases[1].num_anim_frames]);
				} break;
				case TileType::FORT:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 1, 0, &tex_atlases[1].frames[animation_frame % tex_atlases[1].num_anim_frames]);
				} break;
				case TileType::MINE:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 0, 1, &tex_atlases[1].frames[animation_frame % tex_atlases[1].num_anim_frames]);
				} break;
				case TileType::RAIL:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 1, 1, &tex_atlases[1].frames[animation_frame % tex_atlases[1].num_anim_frames]);
				} break;
				default: break;
			}
		}
	}
}
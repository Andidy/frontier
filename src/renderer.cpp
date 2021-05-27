#include "renderer.h"

extern CycleCounter global_cycle_counter;
extern Bitmap blue_noise_tex;

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
			if (a == 0) {
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
	BeginTimer(CT_DRAW_PIXEL);
	if (x < 0 || x > bitmap->width || y < 0 || y > bitmap->height || bitmap->bpp != 4) {
		return false;
	}
	
	uint32_t* buffer = (uint32_t*)bitmap->buffer;
	if (color.a) {
		buffer[x + bitmap->width * y] = (color.b | color.g << 8 | color.r << 16 | color.a << 24);
	}

	EndTimer(CT_DRAW_PIXEL);
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
	BeginTimer(CT_DRAW_RECT);
	if (x < 0 || (x + w) > bitmap->width || y < 0 || (y + h) > bitmap->height ||
		w < 0 || h < 0 || bitmap->bpp != 4) {
		return false;
	}

	uint32_t* buffer = (uint32_t*)bitmap->buffer;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (color.a) {
				buffer[(x + j) + bitmap->width * (y + i)] = (color.b | color.g << 8 | color.r << 16 | color.a << 24);
			}
		}
	}

	EndTimer(CT_DRAW_RECT);
	return true;
}

/*
	Draws the sprite at the given x,y position into the bitmap.
	Bitmap coordinates are 0,0 is top left, width,height is bottom right.
	Returns true if the operation succeeded, false if any section of the
	rect is outside the boundaries of the bitmap.
*/
bool DrawSprite(Bitmap* bitmap, int32_t x, int32_t y, Bitmap* sprite) {
	BeginTimer(CT_DRAW_SPRITE);
	if (x < 0 || (x+sprite->width) > bitmap->width || y < 0 || (y+sprite->height) > bitmap->height ||
		bitmap->bpp != 4 || sprite->bpp != 4) {
		return false;
	}

	uint32_t* bitmap_buffer = (uint32_t*)bitmap->buffer;
	uint32_t* sprite_buffer = (uint32_t*)sprite->buffer;
	for (int i = 0; i < sprite->height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			uint32_t pixel = sprite_buffer[j + sprite->width * i];
			uchar alpha = (uchar)(pixel >> 24);
			if (alpha) {
				bitmap_buffer[(x + j) + bitmap->width * (y + i)] = pixel;
			}
		}
	}

	EndTimer(CT_DRAW_SPRITE);
	return true;
}

/*
	Works the same as draw sprite, except that every pixel in the source
	image is scaled by the scale parameter. So with a scale parameter of
	4 a single pixel in the source image becomes a 4x4 rectangle in the
	bitmap.
*/
bool DrawSpriteMagnified(Bitmap* bitmap, int32_t x, int32_t y, int32_t scale, Bitmap* sprite) {
	BeginTimer(CT_DRAW_SPRITE_MAG);
	if (x < 0 || (x + scale * sprite->width) > bitmap->width || y < 0 || (y + scale * sprite->height) > bitmap->height ||
		bitmap->bpp != 4 || sprite->bpp != 4) {
		return false;
	}

	uint32_t* bitmap_buffer = (uint32_t*)bitmap->buffer;
	uint32_t* sprite_buffer = (uint32_t*)sprite->buffer;
	for (int i = 0; i < sprite->height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			uint32_t pixel = sprite_buffer[j + sprite->width * i];
			for (int k = 0; k < scale * scale; k++) {
				uchar alpha = (uchar)(pixel >> 24);
				if (alpha) {
					bitmap_buffer[(x + k % scale + j * scale) + bitmap->width * (y + k / scale + i * scale)] = pixel;
				}
			}
		}
	}

	EndTimer(CT_DRAW_SPRITE_MAG);
	return true;
}

void DrawUIRect(Bitmap* viewport, int32_t x, int32_t y, int32_t width, int32_t height, int32_t line_width, Color background_color, Color line_color) {
	BeginTimer(CT_UI_DRAW_RECT);
	DrawRect(viewport, x, y, width, height, background_color);
	if (line_width > 0) {
		int32_t lw2 = (2 * line_width);
		DrawRect(viewport, x + line_width, y + line_width, width - lw2, line_width, line_color);
		DrawRect(viewport, x + line_width, y + line_width, line_width, height - lw2, line_color);
		DrawRect(viewport, x + width - lw2, y + line_width, line_width, height - lw2, line_color);
		DrawRect(viewport, x + line_width, y + height - lw2, width - lw2, line_width, line_color);
	}
	EndTimer(CT_UI_DRAW_RECT);
}

void DrawUIText(Bitmap* viewport, int32_t x, int32_t y, char* text, int32_t text_len, Bitmap* font) {
	BeginTimer(CT_UI_DRAW_TEXT);
	const int character_width = 9;
	const int character_height = 16;

	int font_chars_width = font->width / character_width;
	//int font_chars_height = font->height / character_width;

	int32_t start_cursor_x = x;
	int32_t start_cursor_y = y;

	int32_t cursor_x = start_cursor_x;
	int32_t cursor_y = start_cursor_y;

	uint32_t* viewport_buffer = (uint32_t*)viewport->buffer;
	uint32_t* font_buffer = (uint32_t*)font->buffer;

	for (int i = 0; i < text_len; i++) {
		char c = text[i];

		// handle \t
		if (c == 9) {
			cursor_x += character_width * 4;
			continue;
		}
		// handle \n
		if (c == 10) {
			cursor_x = start_cursor_x;
			cursor_y += character_height;
			continue;
		}

		int char_x = c % font_chars_width;
		int char_y = c / font_chars_width;
		int offset_x = char_x * character_width;
		int offset_y = char_y * character_height;
		for (int j = 0; j < character_height; j++) {
			for (int i = 0; i < character_width; i++) {
				int pixel_x = i + offset_x;
				int pixel_y = j + offset_y;
				uint32_t pixel = font_buffer[pixel_x + font->width * pixel_y];
				if ((uchar)(pixel >> 24)) {
					viewport_buffer[(cursor_x + i) + viewport->width * (cursor_y + j)] = pixel;
				}
			}
		}
		cursor_x += character_width;
	}
	EndTimer(CT_UI_DRAW_TEXT);
}

void DrawUIText(Bitmap* viewport, int32_t x, int32_t y, char* text, int32_t text_len, Bitmap* font, Color color) {
	BeginTimer(CT_UI_DRAW_TEXT_COLOR);
	const int character_width = 9;
	const int character_height = 16;

	int font_chars_width = font->width / character_width;
	//int font_chars_height = font->height / character_width;

	int32_t start_cursor_x = x;
	int32_t start_cursor_y = y;

	int32_t cursor_x = start_cursor_x;
	int32_t cursor_y = start_cursor_y;

	uint32_t* viewport_buffer = (uint32_t*)viewport->buffer;
	uint32_t* font_buffer = (uint32_t*)font->buffer;

	for (int i = 0; i < text_len; i++) {
		char c = text[i];

		// handle \t
		if (c == 9) {
			cursor_x += character_width * 4;
			continue;
		}
		// handle \n
		if (c == 10) {
			cursor_x = start_cursor_x;
			cursor_y += character_height;
			continue;
		}

		int char_x = c % font_chars_width;
		int char_y = c / font_chars_width;
		int offset_x = char_x * character_width;
		int offset_y = char_y * character_height;
		for (int j = 0; j < character_height; j++) {
			for (int i = 0; i < character_width; i++) {
				int pixel_x = i + offset_x;
				int pixel_y = j + offset_y;
				uint32_t pixel = font_buffer[pixel_x + font->width * pixel_y];
				uchar r = (uchar)pixel;
				uchar g = (uchar)(pixel >> 8);
				uchar b = (uchar)(pixel >> 16);
				uchar a = (uchar)(pixel >> 24);
				if (a && (r || g || b)) {
					viewport_buffer[(cursor_x + i) + viewport->width * (cursor_y + j)] = (color.b | color.g << 8 | color.r << 16 | color.a << 24);
				}
			}
		}
		cursor_x += character_width;
	}
	EndTimer(CT_UI_DRAW_TEXT_COLOR);
}

void DrawUIText(Bitmap* viewport, int32_t x, int32_t y, char* text, int32_t text_len, Bitmap* font, Color text_color, Color background_color) {
	BeginTimer(CT_UI_DRAW_TEXT_COLOR_BACKGROUND);
	const int character_width = 9;
	const int character_height = 16;

	int font_chars_width = font->width / character_width;
	//int font_chars_height = font->height / character_width;

	int32_t start_cursor_x = x;
	int32_t start_cursor_y = y;

	int32_t cursor_x = start_cursor_x;
	int32_t cursor_y = start_cursor_y;

	uint32_t* viewport_buffer = (uint32_t*)viewport->buffer;
	uint32_t* font_buffer = (uint32_t*)font->buffer;

	for (int i = 0; i < text_len; i++) {
		char c = text[i];

		// handle \t
		if (c == 9) {
			cursor_x += character_width * 4;
			continue;
		}
		// handle \n
		if (c == 10) {
			cursor_x = start_cursor_x;
			cursor_y += character_height;
			continue;
		}

		int char_x = c % font_chars_width;
		int char_y = c / font_chars_width;
		int offset_x = char_x * character_width;
		int offset_y = char_y * character_height;
		for (int j = 0; j < character_height; j++) {
			for (int i = 0; i < character_width; i++) {
				int pixel_x = i + offset_x;
				int pixel_y = j + offset_y;
				uint32_t pixel = font_buffer[pixel_x + font->width * pixel_y];
				uchar r = (uchar)pixel;
				uchar g = (uchar)(pixel >> 8);
				uchar b = (uchar)(pixel >> 16);
				uchar a = (uchar)(pixel >> 24);
				if (a && (r || g || b)) {
					viewport_buffer[(cursor_x + i) + viewport->width * (cursor_y + j)] = (text_color.b | text_color.g << 8 | text_color.r << 16 | text_color.a << 24);
				}
				else {
					viewport_buffer[(cursor_x + i) + viewport->width * (cursor_y + j)] = (background_color.b | background_color.g << 8 | background_color.r << 16 | background_color.a << 24);
				}
			}
		}
		cursor_x += character_width;
	}
	EndTimer(CT_UI_DRAW_TEXT_COLOR_BACKGROUND);
}

// ============================================================================

TilemapRenderer::TilemapRenderer() {}

TilemapRenderer::TilemapRenderer(int tile_w, int tile_h, int tile_s, int v_x, int v_y, int v_w, int v_h, int anim_max_frames, f32 anim_frame_time, Bitmap bitmap) {
	tile_width = tile_w;
	tile_height = tile_h;
	tile_scale = tile_s;
	view_x = v_x;
	view_y = v_y;
	view_w = v_w;
	view_h = v_h;
	view_bitmap.buffer = bitmap.buffer;
	view_bitmap.width = bitmap.width;
	view_bitmap.height = bitmap.height;
	view_bitmap.bpp = bitmap.bpp;
	animation_frame = 0;
	animation_max_frames = anim_max_frames;
	animation_frame_time = 0;
	animation_max_frame_time = anim_frame_time;

	for (int i = 0; i < num_terrain_atlases; i++) {
		terrain_atlases[i].frames = NULL;
		terrain_atlases[i].num_anim_frames = 1;
		terrain_atlases[i].num_subtile_variants = 1;
	}
	for (int i = 0; i < num_feature_atlases; i++) {
		feature_atlases[i].frames = NULL;
		feature_atlases[i].num_anim_frames = 1;
		feature_atlases[i].num_subtile_variants = 1;
	}
	for (int i = 0; i < num_structure_atlases; i++) {
		structure_atlases[i].frames = NULL;
		structure_atlases[i].num_anim_frames = 1;
		structure_atlases[i].num_subtile_variants = 1;
	}
	for (int i = 0; i < num_unit_atlases; i++) {
		unit_atlases[i].frames = NULL;
		unit_atlases[i].num_anim_frames = 1;
		unit_atlases[i].num_subtile_variants = 1;
	}
}

/*
	Resize the viewport.
	Params:
	width: new width of the viewport
	height: new height of the viewport
*/
void TilemapRenderer::ResizeViewport(int width, int height) {
	if (view_bitmap.buffer) {
		free((void*)view_bitmap.buffer);
	}
	
	view_w = width;
	view_h = height;

	view_bitmap.buffer = (uchar*)calloc(view_bitmap.bpp * width * height, sizeof(uchar));
	view_bitmap.width = width;
	view_bitmap.height = height;
}


/*
	Draw a sprite into the viewport.
	Params:
	world_x / world_y: world coordinates of the sprite being drawn
	tex_atlas_x / tex_atlas_y: the index inside the texture atlas of the sprite
	texture_atlas: the texture atlas where the sprite is located
*/
void TilemapRenderer::DrawSprite(int32_t world_x, int32_t world_y, int32_t tex_atlas_x, int32_t tex_atlas_y, Bitmap* texture_atlas) {
	BeginTimer(CT_TM_DRAW_SPRITE);

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

			if (viewport_y < view_bitmap.height && viewport_x < view_bitmap.width) {
				int32_t pixel_x = ((x - world_x) / tile_scale + tex_atlas_off_x);
				int32_t pixel_y = ((y - world_y) / tile_scale + tex_atlas_off_y);
				uint32_t pixel = sprite_buffer[pixel_x + texture_atlas->width * pixel_y];

				uchar alpha = (uchar)(pixel >> 24);
				if(alpha){
					bitmap_buffer[viewport_x + view_bitmap.width * viewport_y] = pixel;
				}
			}
		}
	}
	EndTimer(CT_TM_DRAW_SPRITE);
}

void TilemapRenderer::DrawSubTile(int32_t world_x, int32_t world_y, int32_t tex_atlas_x, int32_t tex_atlas_y, Bitmap* texture_atlas) {
	BeginTimer(CT_TM_DRAW_SUBTILES);

	uint32_t* bitmap_buffer = (uint32_t*)view_bitmap.buffer;
	uint32_t* sprite_buffer = (uint32_t*)texture_atlas->buffer;
	
	// for starting corner we want the larger value
	int32_t start_x = (world_x >= view_x) ? world_x : view_x;
	int32_t start_y = (world_y >= view_y) ? world_y : view_y;
	// for ending corner we want the smaller value
	int32_t end_x = (world_x + (tile_width / 2) * tile_scale < view_x + view_w) ? world_x + (tile_width / 2) * tile_scale : view_x + view_w;
	int32_t end_y = (world_y + (tile_width / 2) * tile_scale < view_y + view_h) ? world_y + (tile_width / 2) * tile_scale : view_y + view_h;

	int32_t tex_atlas_off_x = tex_atlas_x * (tile_width / 2);
	int32_t tex_atlas_off_y = tex_atlas_y * (tile_height / 2);

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			int32_t viewport_x = (x - view_x);
			int32_t viewport_y = (y - view_y);

			if (viewport_y < view_bitmap.height && viewport_x < view_bitmap.width) {
				int32_t pixel_x = ((x - world_x) / tile_scale + tex_atlas_off_x);
				int32_t pixel_y = ((y - world_y) / tile_scale + tex_atlas_off_y);
				uint32_t pixel = sprite_buffer[pixel_x + texture_atlas->width * pixel_y];

				bitmap_buffer[viewport_x + view_bitmap.width * viewport_y] = pixel;
			}
		}
	}

	EndTimer(CT_TM_DRAW_SUBTILES);
}

void TilemapRenderer::DrawSubTiles(int32_t x, int32_t y, int* subtiles, int* variants, Bitmap* texture_atlas) {
	int32_t scaled_tile_width = tile_width * tile_scale;
	int32_t scaled_tile_height = tile_height * tile_scale;

	int32_t x1 = x * scaled_tile_width;
	int32_t x2 = x * scaled_tile_width + (scaled_tile_width / 2);
	int32_t y1 = y * scaled_tile_height;
	int32_t y2 = y * scaled_tile_height + (scaled_tile_height / 2);

	DrawSubTile(x1, y1, subtiles[0], variants[0], texture_atlas);
	DrawSubTile(x2, y1, subtiles[1], variants[1], texture_atlas);
	DrawSubTile(x1, y2, subtiles[2], variants[2], texture_atlas);
	DrawSubTile(x2, y2, subtiles[3], variants[3], texture_atlas);
}

Bitmap* TilemapRenderer::GetTerrainAtlas(TileTerrain type) {
	int index = (int)type;
	return &terrain_atlases[index].frames[animation_frame % terrain_atlases[index].num_anim_frames];
}

Bitmap* TilemapRenderer::GetFeatureAtlas(TileFeature type) {
	int index = (int)type;
	return &feature_atlases[index].frames[animation_frame % feature_atlases[index].num_anim_frames];
}

Bitmap* TilemapRenderer::GetStructureAtlas(TileStructureType type) {
	int index = (int)type;
	return &structure_atlases[index].frames[animation_frame % structure_atlases[index].num_anim_frames];
}

Bitmap* TilemapRenderer::GetUnitAtlas(UnitType type) {
	int index = (int)type;
	return &unit_atlases[index].frames[animation_frame % unit_atlases[index].num_anim_frames];
}

void TilemapRenderer::DrawTilemap(Tilemap* tilemap) {
	BeginTimer(CT_TM_DRAW_TILEMAP);

	int32_t scaled_tile_width = tile_width * tile_scale;
	int32_t scaled_tile_height = tile_height * tile_scale;

	int start_x = view_x / scaled_tile_width;
	int start_y = view_y / scaled_tile_height;
	int end_x = (view_x + view_w) / scaled_tile_width;
	int end_y = (view_y + view_h) / scaled_tile_height;

	for (int y = start_y; y <= end_y; y++) {
		for (int x = start_x; x <= end_x; x++) {
			Tile tile = tilemap->tiles[x + tilemap->width * y];
			if (tile.terrain != TileTerrain::NONE) { 
				DrawSubTiles(x, y, tile.terrain_subtiles, tile.terrain_variants, GetTerrainAtlas(tile.terrain));
			}
			else {
				DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 0, 0, &background_grid);
			}
			//if (tile.feature != TileFeature::NONE) {
			//
			//}
			switch (tile.structure) {
				case TileStructureType::NONE: break;
				case TileStructureType::FARMHOUSE:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 2, 0, &structure_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
				} break;
				case TileStructureType::FIELD:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 0, 0, &structure_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
				} break;
				case TileStructureType::ORCHARD:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 1, 0, &structure_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
				} break;
				case TileStructureType::WOODCUTTER:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 0, 1, &structure_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
				} break;
				case TileStructureType::TEST:
				{
					DrawSprite(x * scaled_tile_width, y * scaled_tile_height, 3, 0, &structure_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
				} break;
				default: break;
			}
		}
	}

	for (int i = 0; i < tilemap->num_units; i++) {
		Unit* unit = &(tilemap->units[i]);
		switch (unit->type) {
			case UnitType::ARMY: 
			{
				DrawSprite(unit->pos_x * scaled_tile_width, unit->pos_y * scaled_tile_height, 0, 0, &unit_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
			} break;
			case UnitType::NAVY:
			{
				DrawSprite(unit->pos_x * scaled_tile_width, unit->pos_y * scaled_tile_height, 1, 0, &unit_atlases[0].frames[animation_frame % unit_atlases[0].num_anim_frames]);
			} break;
			default: break;
		}
	}
	
	EndTimer(CT_TM_DRAW_TILEMAP);
}

enum class SubTile {
	TOP_LEFT,
	TOP,
	TOP_RIGHT,
	LEFT,
	CENTER,
	RIGHT,
	BOTTOM_LEFT,
	BOTTOM,
	BOTTOM_RIGHT,
	TOP_LEFT_INVERSE,
	TOP_RIGHT_INVERSE,
	BOTTOM_LEFT_INVERSE,
	BOTTOM_RIGHT_INVERSE,
	NUM_SUBTILES
};

/*
	Blue noise pulled from provided bitmap. If x or y exceeds dimensions of the
	texture than we wrap to the other side of the texture.
	params: x,y where to sample from, bitmap the bluenoise texture.
*/
int BlueNoise(int32_t x, int32_t y) {
	x %= blue_noise_tex.width;
	y %= blue_noise_tex.height;

	int result = blue_noise_tex.buffer[x + blue_noise_tex.width * y];
	return result;
}

/*
	Cache the wang blob adjacency decisions to determine tile graphics.
	Params: the tilemap we want to cache the results of
*/
void TilemapRenderer::CacheTileRenderingSubtiles(Tilemap* tm) {
	BeginTimer(CT_CACHE_SUBTILES);

	const f32 E = 0.0001f;

	for (int y = 0; y < tm->height; y++) {
		for (int x = 0; x < tm->width; x++) {
			// current tile is at x, y
			// cache subtile variant based on tile position & noise

			int type = (int)tm->tiles[x + tm->width * y].terrain;
			int num_terrain_variants = terrain_atlases[type].num_subtile_variants;
			type = (int)tm->tiles[x + tm->width * y].feature;
			int num_feature_variants = feature_atlases[type].num_subtile_variants;
			type = (int)tm->tiles[x + tm->width * y].structure;
			int num_structure_variants = structure_atlases[type].num_subtile_variants;

			if (tm->tiles[x + tm->width * y].terrain_variant_fixed) {
				int result = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 0) / 256.0f) * ((f32)num_terrain_variants - E));
				tm->tiles[x + tm->width * y].terrain_variants[0] = result;
				tm->tiles[x + tm->width * y].terrain_variants[1] = result;
				tm->tiles[x + tm->width * y].terrain_variants[2] = result;
				tm->tiles[x + tm->width * y].terrain_variants[3] = result;
				
				/*
				result = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 0) / 256.0f) * ((f32)num_feature_variants - E));
				tm->tiles[x + tm->width * y].feature_variants[0] = result;
				tm->tiles[x + tm->width * y].feature_variants[1] = result;
				tm->tiles[x + tm->width * y].feature_variants[2] = result;
				tm->tiles[x + tm->width * y].feature_variants[3] = result;
				
				result = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 0) / 256.0f) * ((f32)num_structure_variants - E));
				tm->tiles[x + tm->width * y].structure_variants[0] = result;
				tm->tiles[x + tm->width * y].structure_variants[1] = result;
				tm->tiles[x + tm->width * y].structure_variants[2] = result;
				tm->tiles[x + tm->width * y].structure_variants[3] = result;
				*/
			}
			else {
				tm->tiles[x + tm->width * y].terrain_variants[0] = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 0) / 256.0f) * ((f32)num_terrain_variants - E));
				tm->tiles[x + tm->width * y].terrain_variants[1] = (int)floorf(((f32)BlueNoise(2 * x + 1, 2 * y + 0) / 256.0f) * ((f32)num_terrain_variants - E));
				tm->tiles[x + tm->width * y].terrain_variants[2] = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 1) / 256.0f) * ((f32)num_terrain_variants - E));
				tm->tiles[x + tm->width * y].terrain_variants[3] = (int)floorf(((f32)BlueNoise(2 * x + 1, 2 * y + 1) / 256.0f) * ((f32)num_terrain_variants - E));

				/*
				tm->tiles[x + tm->width * y].feature_variants[0] = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 0) / 256.0f) * ((f32)num_feature_variants - E));
				tm->tiles[x + tm->width * y].feature_variants[1] = (int)floorf(((f32)BlueNoise(2 * x + 1, 2 * y + 0) / 256.0f) * ((f32)num_feature_variants - E));
				tm->tiles[x + tm->width * y].feature_variants[2] = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 1) / 256.0f) * ((f32)num_feature_variants - E));
				tm->tiles[x + tm->width * y].feature_variants[3] = (int)floorf(((f32)BlueNoise(2 * x + 1, 2 * y + 1) / 256.0f) * ((f32)num_feature_variants - E));
				
				tm->tiles[x + tm->width * y].structure_variants[0] = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 0) / 256.0f) * ((f32)num_structure_variants - E));
				tm->tiles[x + tm->width * y].structure_variants[1] = (int)floorf(((f32)BlueNoise(2 * x + 1, 2 * y + 0) / 256.0f) * ((f32)num_structure_variants - E));
				tm->tiles[x + tm->width * y].structure_variants[2] = (int)floorf(((f32)BlueNoise(2 * x + 0, 2 * y + 1) / 256.0f) * ((f32)num_structure_variants - E));
				tm->tiles[x + tm->width * y].structure_variants[3] = (int)floorf(((f32)BlueNoise(2 * x + 1, 2 * y + 1) / 256.0f) * ((f32)num_structure_variants - E));
				*/
			}

			// cache subtile type
			// index 0 = terrain, 1 = feature, 2 = structure
			int tl[3] = { 0, 0, 0 };
			int t[3] = { 0, 0, 0 };
			int tr[3] = { 0, 0, 0 };
			int l[3] = { 0, 0, 0 };
			int r[3] = { 0, 0, 0 };
			int bl[3] = { 0, 0, 0 };
			int b[3] = { 0, 0, 0 };
			int br[3] = { 0, 0, 0 };
			// check if neighbors are valid, we assume invalid neighbors
			// are matching the tile type

			// Neighbor deltas:
			// -1, -1 | 0, -1 | +1, -1
			// -------+-------+-------
			// -1,  0 | 0,  0 | +1,  0
			// -------+-------+-------
			// -1, +1 | 0, +1 | +1, +1

			// top
			if (ValidNeighbor(tm, x, y, x, y - 1)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x)+tm->width * (y - 1)].terrain) {
					t[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x)+tm->width * (y - 1)].feature) {
					t[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x)+tm->width * (y - 1)].structure) {
					t[2] = 1;
				}
			}
			else {
				t[0] = 1;
				t[1] = 1;
				t[2] = 1;
			}
			// left
			if (ValidNeighbor(tm, x, y, x - 1, y)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x - 1) + tm->width * (y)].terrain) {
					l[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x - 1) + tm->width * (y)].feature) {
					l[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x - 1) + tm->width * (y)].structure) {
					l[2] = 1;
				}
			}
			else {
				l[0] = 1;
				l[1] = 1;
				l[2] = 1;
			}
			// right
			if (ValidNeighbor(tm, x, y, x + 1, y)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x + 1) + tm->width * (y)].terrain) {
					r[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x + 1) + tm->width * (y)].feature) {
					r[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x + 1) + tm->width * (y)].structure) {
					r[2] = 1;
				}
			}
			else {
				r[0] = 1;
				r[1] = 1;
				r[2] = 1;
			}
			// bottom
			if (ValidNeighbor(tm, x, y, x, y + 1)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x)+tm->width * (y + 1)].terrain) {
					b[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x)+tm->width * (y + 1)].feature) {
					b[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x)+tm->width * (y + 1)].structure) {
					b[2] = 1;
				}
			}
			else {
				b[0] = 1;
				b[1] = 1;
				b[2] = 1;
			}

			// corners only matter if both adjacent sides are solid so we do additional checks
			// top left
			if (ValidNeighbor(tm, x, y, x - 1, y - 1)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x - 1) + tm->width * (y - 1)].terrain) {
					tl[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x - 1) + tm->width * (y - 1)].feature) {
					tl[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x - 1) + tm->width * (y - 1)].structure) {
					tl[2] = 1;
				}
			}
			else {
				tl[0] = 1;
				tl[1] = 1;
				tl[2] = 1;
			}
			// top right
			if (ValidNeighbor(tm, x, y, x + 1, y - 1)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x + 1) + tm->width * (y - 1)].terrain) {
					tr[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x + 1) + tm->width * (y - 1)].feature) {
					tr[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x + 1) + tm->width * (y - 1)].structure) {
					tr[2] = 1;
				}
			}
			else {
				tr[0] = 1;
				tr[1] = 1;
				tr[2] = 1;
			}
			// bottom left
			if (ValidNeighbor(tm, x, y, x - 1, y + 1)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x - 1) + tm->width * (y + 1)].terrain) {
					bl[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x - 1) + tm->width * (y + 1)].feature) {
					bl[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x - 1) + tm->width * (y + 1)].structure) {
					bl[2] = 1;
				}
			}
			else {
				bl[0] = 1;
				bl[1] = 1;
				bl[2] = 1;
			}
			// bottom right
			if (ValidNeighbor(tm, x, y, x + 1, y + 1)) {
				if (tm->tiles[x + tm->width * y].terrain == tm->tiles[(x + 1) + tm->width * (y + 1)].terrain) {
					br[0] = 1;
				}
				if (tm->tiles[x + tm->width * y].feature == tm->tiles[(x + 1) + tm->width * (y + 1)].feature) {
					br[1] = 1;
				}
				if (tm->tiles[x + tm->width * y].structure == tm->tiles[(x + 1) + tm->width * (y + 1)].structure) {
					br[2] = 1;
				}
			}
			else {
				br[0] = 1;
				br[1] = 1;
				br[2] = 1;
			}

			// now that we know which neighbors match the tile type
			// we can determine the sub tiles

			int* subtile_0 = &(tm->tiles[x + tm->width * y].terrain_subtiles[0]);
			int* subtile_1 = &(tm->tiles[x + tm->width * y].terrain_subtiles[1]);
			int* subtile_2 = &(tm->tiles[x + tm->width * y].terrain_subtiles[2]);
			int* subtile_3 = &(tm->tiles[x + tm->width * y].terrain_subtiles[3]);

			// top left subtile 0
			int index = t[0] + 2 * tl[0] + 4 * l[0];

			if (index == 0 || index == 2) {
				*subtile_0 = (int)SubTile::TOP_LEFT;
			}
			else if (index == 1 || index == 3) {
				*subtile_0 = (int)SubTile::LEFT;
			}
			else if (index == 4 || index == 6) {
				*subtile_0 = (int)SubTile::TOP;
			}
			else if (index == 5) {
				*subtile_0 = (int)SubTile::TOP_LEFT_INVERSE;
			}
			else if (index == 7) {
				*subtile_0 = (int)SubTile::CENTER;
			}

			// top right subtile 1
			index = r[0] + 2 * tr[0] + 4 * t[0];

			if (index == 0 || index == 2) {
				*subtile_1 = (int)SubTile::TOP_RIGHT;
			}
			else if (index == 1 || index == 3) {
				*subtile_1 = (int)SubTile::TOP;
			}
			else if (index == 4 || index == 6) {
				*subtile_1 = (int)SubTile::RIGHT;
			}
			else if (index == 5) {
				*subtile_1 = (int)SubTile::TOP_RIGHT_INVERSE;
			}
			else if (index == 7) {
				*subtile_1 = (int)SubTile::CENTER;
			}

			// bottom left subtile 2
			index = b[0] + 2 * bl[0] + 4 * l[0];

			if (index == 0 || index == 2) {
				*subtile_2 = (int)SubTile::BOTTOM_LEFT;
			}
			else if (index == 1 || index == 3) {
				*subtile_2 = (int)SubTile::LEFT;
			}
			else if (index == 4 || index == 6) {
				*subtile_2 = (int)SubTile::BOTTOM;
			}
			else if (index == 5) {
				*subtile_2 = (int)SubTile::BOTTOM_LEFT_INVERSE;
			}
			else if (index == 7) {
				*subtile_2 = (int)SubTile::CENTER;
			}

			// bottom right subtile 3
			index = r[0] + 2 * br[0] + 4 * b[0];

			if (index == 0 || index == 2) {
				*subtile_3 = (int)SubTile::BOTTOM_RIGHT;
			}
			else if (index == 1 || index == 3) {
				*subtile_3 = (int)SubTile::BOTTOM;
			}
			else if (index == 4 || index == 6) {
				*subtile_3 = (int)SubTile::RIGHT;
			}
			else if (index == 5) {
				*subtile_3 = (int)SubTile::BOTTOM_RIGHT_INVERSE;
			}
			else if (index == 7) {
				*subtile_3 = (int)SubTile::CENTER;
			}

			/*
			// now the features
			subtile_0 = &(tm->tiles[x + tm->width * y].feature_subtiles[0]);
			subtile_1 = &(tm->tiles[x + tm->width * y].feature_subtiles[1]);
			subtile_2 = &(tm->tiles[x + tm->width * y].feature_subtiles[2]);
			subtile_3 = &(tm->tiles[x + tm->width * y].feature_subtiles[3]);

			// top left subtile 0
			index = t[1] + 2 * tl[1] + 4 * l[1];

			if (index == 0 || index == 2) {
				*subtile_0 = (int)SubTile::TOP_LEFT;
			}
			else if (index == 1 || index == 3) {
				*subtile_0 = (int)SubTile::LEFT;
			}
			else if (index == 4 || index == 6) {
				*subtile_0 = (int)SubTile::TOP;
			}
			else if (index == 5) {
				*subtile_0 = (int)SubTile::TOP_LEFT_INVERSE;
			}
			else if (index == 7) {
				*subtile_0 = (int)SubTile::CENTER;
			}

			// top right subtile 1
			index = r[1] + 2 * tr[1] + 4 * t[1];

			if (index == 0 || index == 2) {
				*subtile_1 = (int)SubTile::TOP_RIGHT;
			}
			else if (index == 1 || index == 3) {
				*subtile_1 = (int)SubTile::TOP;
			}
			else if (index == 4 || index == 6) {
				*subtile_1 = (int)SubTile::RIGHT;
			}
			else if (index == 5) {
				*subtile_1 = (int)SubTile::TOP_RIGHT_INVERSE;
			}
			else if (index == 7) {
				*subtile_1 = (int)SubTile::CENTER;
			}

			// bottom left subtile 2
			index = b[1] + 2 * bl[1] + 4 * l[1];

			if (index == 0 || index == 2) {
				*subtile_2 = (int)SubTile::BOTTOM_LEFT;
			}
			else if (index == 1 || index == 3) {
				*subtile_2 = (int)SubTile::LEFT;
			}
			else if (index == 4 || index == 6) {
				*subtile_2 = (int)SubTile::BOTTOM;
			}
			else if (index == 5) {
				*subtile_2 = (int)SubTile::BOTTOM_LEFT_INVERSE;
			}
			else if (index == 7) {
				*subtile_2 = (int)SubTile::CENTER;
			}

			// bottom right subtile 3
			index = r[1] + 2 * br[1] + 4 * b[1];

			if (index == 0 || index == 2) {
				*subtile_3 = (int)SubTile::BOTTOM_RIGHT;
			}
			else if (index == 1 || index == 3) {
				*subtile_3 = (int)SubTile::BOTTOM;
			}
			else if (index == 4 || index == 6) {
				*subtile_3 = (int)SubTile::RIGHT;
			}
			else if (index == 5) {
				*subtile_3 = (int)SubTile::BOTTOM_RIGHT_INVERSE;
			}
			else if (index == 7) {
				*subtile_3 = (int)SubTile::CENTER;
			}

			// now the structures
			subtile_0 = &(tm->tiles[x + tm->width * y].structure_subtiles[0]);
			subtile_1 = &(tm->tiles[x + tm->width * y].structure_subtiles[1]);
			subtile_2 = &(tm->tiles[x + tm->width * y].structure_subtiles[2]);
			subtile_3 = &(tm->tiles[x + tm->width * y].structure_subtiles[3]);

			// top left subtile 0
			index = t[2] + 2 * tl[2] + 4 * l[2];

			if (index == 0 || index == 2) {
				*subtile_0 = (int)SubTile::TOP_LEFT;
			}
			else if (index == 1 || index == 3) {
				*subtile_0 = (int)SubTile::LEFT;
			}
			else if (index == 4 || index == 6) {
				*subtile_0 = (int)SubTile::TOP;
			}
			else if (index == 5) {
				*subtile_0 = (int)SubTile::TOP_LEFT_INVERSE;
			}
			else if (index == 7) {
				*subtile_0 = (int)SubTile::CENTER;
			}

			// top right subtile 1
			index = r[2] + 2 * tr[2] + 4 * t[2];

			if (index == 0 || index == 2) {
				*subtile_1 = (int)SubTile::TOP_RIGHT;
			}
			else if (index == 1 || index == 3) {
				*subtile_1 = (int)SubTile::TOP;
			}
			else if (index == 4 || index == 6) {
				*subtile_1 = (int)SubTile::RIGHT;
			}
			else if (index == 5) {
				*subtile_1 = (int)SubTile::TOP_RIGHT_INVERSE;
			}
			else if (index == 7) {
				*subtile_1 = (int)SubTile::CENTER;
			}

			// bottom left subtile 2
			index = b[2] + 2 * bl[2] + 4 * l[2];

			if (index == 0 || index == 2) {
				*subtile_2 = (int)SubTile::BOTTOM_LEFT;
			}
			else if (index == 1 || index == 3) {
				*subtile_2 = (int)SubTile::LEFT;
			}
			else if (index == 4 || index == 6) {
				*subtile_2 = (int)SubTile::BOTTOM;
			}
			else if (index == 5) {
				*subtile_2 = (int)SubTile::BOTTOM_LEFT_INVERSE;
			}
			else if (index == 7) {
				*subtile_2 = (int)SubTile::CENTER;
			}

			// bottom right subtile 3
			index = r[2] + 2 * br[2] + 4 * b[2];

			if (index == 0 || index == 2) {
				*subtile_3 = (int)SubTile::BOTTOM_RIGHT;
			}
			else if (index == 1 || index == 3) {
				*subtile_3 = (int)SubTile::BOTTOM;
			}
			else if (index == 4 || index == 6) {
				*subtile_3 = (int)SubTile::RIGHT;
			}
			else if (index == 5) {
				*subtile_3 = (int)SubTile::BOTTOM_RIGHT_INVERSE;
			}
			else if (index == 7) {
				*subtile_3 = (int)SubTile::CENTER;
			}
			*/
		}
	}
	EndTimer(CT_CACHE_SUBTILES);
}
#ifndef RENDERER_H
#define RENDERER_H

#include "universal.h"

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

#endif

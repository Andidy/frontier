#ifndef UNIVERSAL_H
#define UNIVERSAL_H

#include "ady_types.h"
#include "ady_math.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// Macros
#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])

#define Kilobytes(val) (1024 * (val))
#define Megabytes(val) (1024 * Kilobytes(val))
#define Gigabytes(val) (1024 * Megabytes(val))

// end Macros
// ============================================================================
// Debug stuff

void DebugPrint(char* str);

// end Debug stuff
// ============================================================================
// Memory

struct Memory {
	b32 isInitialized;
	u64 size;
	void* data;
};

// This allocator is to be used for resources which will exist for the entire 
// period the program is running
struct PermanentResourceAllocator {
	i64 size;
	i64 offset;
	uchar* backing_buffer;

	PermanentResourceAllocator(i64 size);
	void* Allocate(i64 alloc_size);
	void Free();
	void FreeBackingBuffer();
};

// This allocator is for resources that should only exist for a single loop
// iteration, for example a frame of the game loop, and then the Free()
// method is called at the end of the loop, freeing all contents of the
// allocator but not its backing buffer
typedef PermanentResourceAllocator FrameAllocator;

// end Memory
// ============================================================================
// Input

struct ButtonState {
	i32 transitionCount;
	b32 endedDown;
};

struct ControllerState {
	b32 isAnalog;
	f32 lstartx, lstarty;
	f32 lmaxx, lmaxy;
	f32 lminx, lminy;
	f32 lendx, lendy;

	f32 rstartx, rstarty;
	f32 rmaxx, rmaxy;
	f32 rminx, rminy;
	f32 rendx, rendy;

	union {
		ButtonState buttons[16];
		struct {
			ButtonState a, b, x, y;
			ButtonState up, down, left, right;
			ButtonState l1, r1;
			ButtonState l2, r2;
			ButtonState l3, r3;
			ButtonState start, select;
		};
	};
};

const int NUM_MOUSE_BUTTONS = 5;
struct MouseState {
	int32_t x, y;
	union {
		ButtonState buttons[NUM_MOUSE_BUTTONS];
		struct {
			ButtonState left, right, middle, x1, x2;
		};
	};
};

const int NUM_KEYBOARD_BUTTONS = 32;
struct KeyboardState {
	union {
		ButtonState buttons[NUM_KEYBOARD_BUTTONS];
		struct {
			ButtonState a, b, c, d, e, f, g, h, i, j, k, l,
				m, n, o, p, q, r, s, t, u, v, w, x, y, z;
			ButtonState up, down, left, right;
			ButtonState space, escape;
		};
	};
};

struct Input {
	MouseState mouse;
	KeyboardState keyboard;
};

// ------------ Functions -----------------

inline b32 keyPressed(ButtonState button) {
	return (button.endedDown) && (button.transitionCount > 0);
}

inline b32 keyReleased(ButtonState button) {
	return !(button.endedDown) && (button.transitionCount > 0);
}

inline b32 keyDown(ButtonState button) {
	return (button.endedDown) && (button.transitionCount == 0);
}

// end Input
// ========================================================
// FILE IO

struct debug_ReadFileResult {
	u64 size;
	void* data;
};

debug_ReadFileResult debug_ReadFile(char* filename);
b32 debug_WriteFile(char* filename, u32 memorySize, void* memory);
void debug_FreeFile(void* memory);

// end FILE IO
// ============================================================================

#endif // !UNIVERSAL_H

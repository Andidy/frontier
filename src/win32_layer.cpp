#include "universal.h"

#include "game.h"

#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/stb/stb_image_write.h"

#include "../libs/json11-master/json11.hpp"

#include <windows.h>
#include <windowsx.h>
#include <xinput.h>


#ifndef UNICODE
#define UNICODE
#endif 

int window_width;
int window_height;

int client_width;
int client_height;

bool window_resized = false;

int win32_running = 0;

// ============================================================================
// USER INPUT

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);

X_INPUT_GET_STATE(XInputGetStateStub) {
	return ERROR_DEVICE_NOT_CONNECTED;
}
X_INPUT_SET_STATE(XInputSetStateStub) {
	return ERROR_DEVICE_NOT_CONNECTED;
}

// The assigments are in win32_LoadXInput because C doesn't like the assignments
static x_input_get_state* XInputGetState_; // = XInputGetStateStub;
static x_input_set_state* XInputSetState_; // = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

static void win32_LoadXInput() {
	XInputGetState_ = XInputGetStateStub;
	XInputSetState_ = XInputSetStateStub;

	// If we don't have xinput1.4 check for xinput1.3
	HMODULE XInputLibrary;
	XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!XInputLibrary) {
		XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
	if (!XInputLibrary) {
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if (XInputLibrary) {
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

// // something for controllers?
// internal void win32_ProcessXInputDigitalButton(ButtonState* oldstate, ButtonState* newstate, DWORD buttonBit, DWORD xinputButtonState) {
//  newstate->endedDown = ((xinputButtonState & buttonBit) == buttonBit);
//  newstate->transitionCount = (oldstate->endedDown != newstate->endedDown) ? 1 : 0;
//}

static void win32_ProcessKeyboardMessage(ButtonState* newState, b32 isDown) {
	assert(newState->endedDown != isDown);
	newState->endedDown = isDown;
	newState->transitionCount += 1;
}

static void win32_UpdateInput(Input* gameInput, HWND window) {
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		switch (message.message) {
			case WM_QUIT: win32_running = false; break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				u32 vkCode = (u32)message.wParam;
				b32 wasDown = ((message.lParam & (1 << 30)) != 0);
				b32 isDown = ((message.lParam & (1 << 31)) == 0);

				if (isDown != wasDown) {
					switch (vkCode) {
						case 0x30: win32_ProcessKeyboardMessage(&gameInput->keyboard.zero, isDown); break;
						case 0x31: win32_ProcessKeyboardMessage(&gameInput->keyboard.one, isDown); break;
						case 0x32: win32_ProcessKeyboardMessage(&gameInput->keyboard.two, isDown); break;
						case 0x33: win32_ProcessKeyboardMessage(&gameInput->keyboard.three, isDown); break;
						case 0x34: win32_ProcessKeyboardMessage(&gameInput->keyboard.four, isDown); break;
						case 0x35: win32_ProcessKeyboardMessage(&gameInput->keyboard.five, isDown); break;
						case 0x36: win32_ProcessKeyboardMessage(&gameInput->keyboard.six, isDown); break;
						case 0x37: win32_ProcessKeyboardMessage(&gameInput->keyboard.seven, isDown); break;
						case 0x38: win32_ProcessKeyboardMessage(&gameInput->keyboard.eight, isDown); break;
						case 0x39: win32_ProcessKeyboardMessage(&gameInput->keyboard.nine, isDown); break;
						
						case 'A': win32_ProcessKeyboardMessage(&gameInput->keyboard.a, isDown); break;
						case 'B': win32_ProcessKeyboardMessage(&gameInput->keyboard.b, isDown); break;
						case 'C': win32_ProcessKeyboardMessage(&gameInput->keyboard.c, isDown); break;
						case 'D': win32_ProcessKeyboardMessage(&gameInput->keyboard.d, isDown); break;
						case 'E': win32_ProcessKeyboardMessage(&gameInput->keyboard.e, isDown); break;
						case 'F': win32_ProcessKeyboardMessage(&gameInput->keyboard.f, isDown); break;
						case 'G': win32_ProcessKeyboardMessage(&gameInput->keyboard.g, isDown); break;
						case 'H': win32_ProcessKeyboardMessage(&gameInput->keyboard.h, isDown); break;
						case 'I': win32_ProcessKeyboardMessage(&gameInput->keyboard.i, isDown); break;
						case 'J': win32_ProcessKeyboardMessage(&gameInput->keyboard.j, isDown); break;
						case 'K': win32_ProcessKeyboardMessage(&gameInput->keyboard.k, isDown); break;
						case 'L': win32_ProcessKeyboardMessage(&gameInput->keyboard.l, isDown); break;
						case 'M': win32_ProcessKeyboardMessage(&gameInput->keyboard.m, isDown); break;
						case 'N': win32_ProcessKeyboardMessage(&gameInput->keyboard.n, isDown); break;
						case 'O': win32_ProcessKeyboardMessage(&gameInput->keyboard.o, isDown); break;
						case 'P': win32_ProcessKeyboardMessage(&gameInput->keyboard.p, isDown); break;
						case 'Q': win32_ProcessKeyboardMessage(&gameInput->keyboard.q, isDown); break;
						case 'R': win32_ProcessKeyboardMessage(&gameInput->keyboard.r, isDown); break;
						case 'S': win32_ProcessKeyboardMessage(&gameInput->keyboard.s, isDown); break;
						case 'T': win32_ProcessKeyboardMessage(&gameInput->keyboard.t, isDown); break;
						case 'U': win32_ProcessKeyboardMessage(&gameInput->keyboard.u, isDown); break;
						case 'V': win32_ProcessKeyboardMessage(&gameInput->keyboard.v, isDown); break;
						case 'W': win32_ProcessKeyboardMessage(&gameInput->keyboard.w, isDown); break;
						case 'X': win32_ProcessKeyboardMessage(&gameInput->keyboard.x, isDown); break;
						case 'Y': win32_ProcessKeyboardMessage(&gameInput->keyboard.y, isDown); break;
						case 'Z': win32_ProcessKeyboardMessage(&gameInput->keyboard.z, isDown); break;

						case VK_LBUTTON: win32_ProcessKeyboardMessage(&gameInput->mouse.left, isDown); break;
						case VK_RBUTTON: win32_ProcessKeyboardMessage(&gameInput->mouse.right, isDown); break;
						case VK_MBUTTON: win32_ProcessKeyboardMessage(&gameInput->mouse.middle, isDown); break;
						case VK_XBUTTON1: win32_ProcessKeyboardMessage(&gameInput->mouse.x1, isDown); break;
						case VK_XBUTTON2: win32_ProcessKeyboardMessage(&gameInput->mouse.x2, isDown); break;

						case VK_UP: win32_ProcessKeyboardMessage(&gameInput->keyboard.up, isDown); break;
						case VK_DOWN: win32_ProcessKeyboardMessage(&gameInput->keyboard.down, isDown); break;
						case VK_LEFT: win32_ProcessKeyboardMessage(&gameInput->keyboard.left, isDown); break;
						case VK_RIGHT: win32_ProcessKeyboardMessage(&gameInput->keyboard.right, isDown); break;

						case VK_SPACE: win32_ProcessKeyboardMessage(&gameInput->keyboard.space, isDown); break;
						case VK_ESCAPE: win32_ProcessKeyboardMessage(&gameInput->keyboard.escape, isDown); break;
					}
				}

				b32 altKeyDown = ((message.lParam & (1 << 29)) != 0);
				if ((vkCode == VK_F4) && altKeyDown) {
					win32_running = false;
				}
			} break;

			case WM_LBUTTONDOWN: { 
				gameInput->mouse.left.endedDown = 1; 
				gameInput->mouse.left.transitionCount = 1; 
			} break;
			case WM_LBUTTONUP: { 
				gameInput->mouse.left.endedDown = 0; 
				gameInput->mouse.left.transitionCount = 1; 
			} break;
			case WM_MBUTTONDOWN: { 
				gameInput->mouse.middle.endedDown = 1; 
				gameInput->mouse.middle.transitionCount = 1; 
			} break;
			case WM_MBUTTONUP: { 
				gameInput->mouse.middle.endedDown = 0; 
				gameInput->mouse.middle.transitionCount = 1; 
			} break;
			case WM_RBUTTONDOWN: { 
				gameInput->mouse.right.endedDown = 1; 
				gameInput->mouse.right.transitionCount = 1; 
			} break;
			case WM_RBUTTONUP: { 
				gameInput->mouse.right.endedDown = 0; 
				gameInput->mouse.right.transitionCount = 1; 
			} break;
			case WM_XBUTTONDOWN: { 
				DWORD fwButton = GET_XBUTTON_WPARAM(message.wParam);
				if (fwButton == XBUTTON1) {
					gameInput->mouse.x1.endedDown = 1;
					gameInput->mouse.x1.transitionCount = 1;
				}
				else {
					gameInput->mouse.x2.endedDown = 1;
					gameInput->mouse.x2.transitionCount = 1;
				}
			} break;
			case WM_XBUTTONUP: { 
				DWORD fwButton = GET_XBUTTON_WPARAM(message.wParam);
				if (fwButton == XBUTTON1) {
					gameInput->mouse.x1.endedDown = 0;
					gameInput->mouse.x1.transitionCount = 1;
				}
				else {
					gameInput->mouse.x2.endedDown = 0;
					gameInput->mouse.x2.transitionCount = 1;
				}
			} break;

			case WM_MOUSEMOVE:
			{
				// mouse move is for only when action is needed during movement

				//int32_t xPos = GET_X_LPARAM(message.lParam);
				//int32_t yPos = GET_Y_LPARAM(message.lParam);

				/*
				// Check for if modifer keys are pressed while moving mouse
				uint32_t modifierKeyFlags = message.wParam;
				if (modifierKeyFlags & MK_CONTROL) {}
				if (modifierKeyFlags & MK_LBUTTON) {}
				if (modifierKeyFlags & MK_RBUTTON) {}
				if (modifierKeyFlags & MK_MBUTTON) {}
				if (modifierKeyFlags & MK_SHIFT){}
				if (modifierKeyFlags & MK_XBUTTON1){}
				if (modifierKeyFlags & MK_XBUTTON2) {}
				*/

				//gameInput->mouse.x = xPos;
				//gameInput->mouse.y = yPos;
			} break;

			default:
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			} break;
		}
	}
	POINT cursorPoint;
	if (GetCursorPos(&cursorPoint)) {
		if (ScreenToClient(window, &cursorPoint)) {
			gameInput->mouse.x = cursorPoint.x;
			gameInput->mouse.y = cursorPoint.y;
		}
	}
}

// end USER INPUT
// ============================================================================
// FILE IO

debug_ReadFileResult debug_ReadFile(char* filename) {
	debug_ReadFileResult result = {};
	HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (file != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER fileSize;

		if (GetFileSizeEx(file, &fileSize)) {
			assert(fileSize.QuadPart <= 0xFFFFFFFF);
			result.data = VirtualAlloc(0, (SIZE_T)fileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if (result.data) {
				DWORD bytesRead;
				if (ReadFile(file, result.data, (i32)fileSize.QuadPart, &bytesRead, 0) && (bytesRead == fileSize.QuadPart)) {
					result.size = fileSize.QuadPart;
				}
				else {
					debug_FreeFile(result.data);
					result.data = 0;
				}
			}
		}
		CloseHandle(file);
	}
	return result;
}

b32 debug_WriteFile(char* filename, u32 memorySize, void* memory) {
  b32 result = 0;
  HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

  if (file != INVALID_HANDLE_VALUE) {
	DWORD bytesWritten;

	if (WriteFile(file, memory, memorySize, &bytesWritten, 0)) {
	  result = (bytesWritten == memorySize);
	}
	else {
	  // error
	}
	CloseHandle(file);
  }
  return result;
}

void debug_FreeFile(void* memory) {
	if (memory) {
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

// end FILE IO
// ============================================================================
// Game IO

bool LoadGameSettings(GameState* gs) {
	bool load_success = true;

	debug_ReadFileResult json = debug_ReadFile((char*)"assets/game_settings.json");
	if (json.data != NULL && json.size >= 0) {
		std::string json_err_str;
		json11::Json game_settings_json = json11::Json::parse((char*)json.data, json_err_str);

		gs->tilemap.width = game_settings_json["map_width"].int_value();
		gs->tilemap.height = game_settings_json["map_height"].int_value();
		gs->tilemap.num_units = game_settings_json["num_units"].int_value();
		gs->tilemap.wrap_horz = game_settings_json["map_wrap_horz"].bool_value();
		gs->tilemap.wrap_vert = game_settings_json["map_wrap_vert"].bool_value();
	}
	else {
		load_success = false;
	}
	
	return load_success;
}

// end Game IO
// ============================================================================
// Memory

PermanentResourceAllocator::PermanentResourceAllocator(i64 size) {
	this->size = size;
	this->offset = 0;

	this->backing_buffer = (uchar*)VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void* PermanentResourceAllocator::Allocate(i64 alloc_size) {
	if (offset + alloc_size < size) {
		void* temp = &backing_buffer[offset];
		offset += alloc_size;
		memset(temp, 0, alloc_size);
		return temp;
	}
	OutputDebugStringA("PermanentResourceAllocator ran out of memory\n");
	return NULL;
}

void PermanentResourceAllocator::Free() {
	offset = 0;
}

void PermanentResourceAllocator::FreeBackingBuffer() {
	if (backing_buffer) {
		VirtualFree(backing_buffer, 0, MEM_RELEASE);
	}
	size = 0;
	offset = 0;
}

// end Memory
// ============================================================================
// Graphics

// this is a blue noise texture to be used by anyone that needs it. It should
// not be modified after loading.
Bitmap blue_noise_tex;

// end Graphics
// ============================================================================

void DebugPrint(char* str) {
	OutputDebugStringA(str);
}

CycleCounter global_cycle_counter;

void BeginTimer(CycleType type) {
	global_cycle_counter.start_cycles[type] = __rdtsc();
}

void EndTimer(CycleType type) {
	global_cycle_counter.cycles[type] += __rdtsc() - global_cycle_counter.start_cycles[type];
	global_cycle_counter.times_called[type] += 1;
}

struct win32_WindowDimension {
	i32 width;
	i32 height;
};

static win32_WindowDimension win32_GetWindowDimension(HWND window) {
	RECT clientRect;
	GetClientRect(window, &clientRect);

	win32_WindowDimension windowDimension = {};
	windowDimension.width = clientRect.right - clientRect.left;
	windowDimension.height = clientRect.bottom - clientRect.top;
	return windowDimension;
}

struct win32_OffScreenBuffer {
	// Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
	BITMAPINFO info;
	void* memory;
	i32 width;
	i32 height;
	i32 pitch;
	i32 bytesPerPixel;
};
win32_OffScreenBuffer globalBackBuffer;

static void win32_ResizeDIBSection(win32_OffScreenBuffer* buffer, i32 width, i32 height) {
	if (buffer->memory) {
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;

	int bytesPerPixel = 4;
	buffer->bytesPerPixel = bytesPerPixel;

	// NOTE(casey): When the biHeight field is negative, this is the clue to
	// Windows to treat this bitmap as top-down, not bottom-up, meaning that
	// the first three bytes of the image are the color for the top left pixel
	// in the bitmap, not the bottom left!
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	// NOTE(casey): Thank you to Chris Hecker of Spy Party fame
	// for clarifying the deal with StretchDIBits and BitBlt!
	// No more DC for us.
	int bitmapMemorySize = (buffer->width * buffer->height) * bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = width * bytesPerPixel;
}

static void Win32DisplayBufferInWindow(win32_OffScreenBuffer* buffer,
	HDC hdc, int window_wid, int window_hgt) {
	// NOTE(casey): For prototyping purposes, we're going to always blit
	// 1-to-1 pixels to make sure we don't introduce artifacts with
	// stretching while we are learning to code the renderer!
	StretchDIBits(hdc,
		0, 0, buffer->width, buffer->height,
		0, 0, buffer->width, buffer->height,
		buffer->memory,
		&buffer->info,
		DIB_RGB_COLORS, SRCCOPY);
}

// ============================================================================


LRESULT CALLBACK win32_WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (message) {
		case WM_CREATE:
		{
			OutputDebugStringW(L"WM_CREATE\n");
		} break;
		case WM_SIZE: 
		{
			OutputDebugStringW(L"WM_SIZE\n");
			window_resized = true;
			win32_WindowDimension dim = win32_GetWindowDimension(hwnd);
			win32_ResizeDIBSection(&globalBackBuffer, dim.width, dim.height);

			client_width = dim.width;
			client_height = dim.height;
		} break;
		case WM_DESTROY: { win32_running = 0; OutputDebugStringW(L"WM_DESTROY\n"); } break;
		case WM_CLOSE: { win32_running = 0; OutputDebugStringW(L"WM_CLOSE\n"); } break;
		case WM_ACTIVATEAPP: OutputDebugStringW(L"WM_ACTIVATEAPP\n"); break;
		
		case WM_KEYUP:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
		{
			OutputDebugStringW(L"Keyboard Event in windows callback function\n");
			assert(0);
		} break;
		default:
		{
			result = DefWindowProcW(hwnd, message, wParam, lParam);
		} break;
	}

	return result;
}

int WINAPI wWinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hprevinstance, _In_ LPWSTR pCmdLine, _In_ int nCmdShow) {
	// ============================================================================
	// init cycle counter
	for (int i = 0; i < CT_NUM_CYCLE_TYPES; i++) {
		global_cycle_counter.start_cycles[i] = 0;
		global_cycle_counter.cycles[i] = 0;
		global_cycle_counter.times_called[i] = 0;
	}
	
	// done initializing cycle counter
	// ============================================================================

	win32_LoadXInput();
	
	// ============================================================================
	// Timing Info

	LARGE_INTEGER perftimerfreqresult;
	QueryPerformanceFrequency(&perftimerfreqresult);
	i64 perftimerfreq = perftimerfreqresult.QuadPart;

	// done initializing timing info
	// ============================================================================
	// Load settings
	
	debug_ReadFileResult settings_file = debug_ReadFile((char*)"assets/settings.json");
	std::string json_err_str;
	json11::Json json = json11::Json::parse((char*)settings_file.data, json_err_str);

	// bool fullscreen = json["fullscreen"].bool_value();

	client_width = json["window_width"].int_value();
	client_height = json["window_height"].int_value();

	// Add 16 to width and 39 to height so that the client area is the numbers you
	// actually want
	window_width = client_width + 16;
	window_height = client_height + 39;

	// done loading settings
	// ============================================================================

	{
		int w, h, n;
		uchar* buf = stbi_load("assets/blue_noise.png", &w, &h, &n, 1);
		blue_noise_tex.buffer = buf;
		blue_noise_tex.width = w;
		blue_noise_tex.height = h;
		blue_noise_tex.bpp = 1;
	}	

	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = win32_WindowCallback;
	window_class.hInstance = hinstance;
	// window_class.hIcon = ;
	window_class.lpszClassName = L"FrontierWindowClass";

	if (RegisterClass(&window_class)) {
		HWND window = CreateWindowExW(
			0, window_class.lpszClassName, L"Game",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			window_width, window_height, 0, 0, hinstance, 0
		);

		if (window) {
			win32_WindowDimension dim = win32_GetWindowDimension(window);
			win32_ResizeDIBSection(&globalBackBuffer, dim.width, dim.height);
			
			char str_buffer[256];
			sprintf_s(str_buffer, "Client x,y: %d, %d\n", dim.width, dim.height);
			DebugPrint(str_buffer);

			win32_running = 1;

			LARGE_INTEGER lasttimer;
			QueryPerformanceCounter(&lasttimer);
			u64 lastcyclecount = __rdtsc();

			Input gameInput[2] = {};
			Input* newInput = &gameInput[0];
			Input* oldInput = &gameInput[1];

			Memory gameMemory = {};
			gameMemory.isInitialized = 0;
			gameMemory.size = Kilobytes((u64)1);
			gameMemory.data = VirtualAlloc(0, (SIZE_T)gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			InitGameState(&gameMemory);

			Bitmap viewport = { (uchar*)globalBackBuffer.memory, globalBackBuffer.width, globalBackBuffer.height, 4 };
			
			int32_t game_viewport_width = 32 * 32;
			int32_t game_viewport_height = 32 * 24;

			Bitmap game_viewport = { NULL, 0, 0, 0 };
			game_viewport.width = game_viewport_width;
			game_viewport.height = game_viewport_height;
			game_viewport.bpp = 4;
			game_viewport.buffer = (uchar*)calloc(game_viewport.bpp * game_viewport.width * game_viewport.height, sizeof(uchar));

			TilemapRenderer tilemap_renderer(32, 32, 1, 0, 0, game_viewport.width, game_viewport.height, 4, 0.25f, game_viewport);
			
			// testing
			{
				debug_ReadFileResult texture_defines = debug_ReadFile((char*)"assets/textures.json");
				json11::Json textures_json = json11::Json::parse((char*)texture_defines.data, json_err_str);

				int num_tex_atlases = 0;
				while (textures_json[num_tex_atlases].is_object()) {
					num_tex_atlases += 1;
				}

				//tilemap_renderer.num_tex_atlases_json = num_tex_atlases;
				//tilemap_renderer.tex_atlases_json = (TextureAtlas*)calloc(num_tex_atlases, sizeof(TextureAtlas));

				char buffer[256];
				int w = 0, h = 0, n = 0;
				for (int i = 0; i < num_tex_atlases; i++) {
					std::string file_name = textures_json[i]["name"].string_value();
					int type = textures_json[i]["type"].int_value();
					int index = textures_json[i]["index"].int_value();

					// determine which texture atlas array we are placing the texture atlas into
					if (type == 0) {
						int num_anim_frames = textures_json[i]["animation_frames"].int_value();
						tilemap_renderer.terrain_atlases[index].num_anim_frames = num_anim_frames;
						tilemap_renderer.terrain_atlases[index].num_subtile_variants = textures_json[i]["subtile_variants"].int_value();

						tilemap_renderer.terrain_atlases[index].frames = (Bitmap*)calloc(num_anim_frames, sizeof(Bitmap));

						for (int j = 0; j < num_anim_frames; j++) {
							snprintf(buffer, 256, "assets/%s_%d.bmp", file_name.c_str(), j);
							uchar* buf = stbi_load(buffer, &w, &h, &n, 4);
							tilemap_renderer.terrain_atlases[index].frames[j].buffer = buf;
							tilemap_renderer.terrain_atlases[index].frames[j].width = w;
							tilemap_renderer.terrain_atlases[index].frames[j].height = h;
							tilemap_renderer.terrain_atlases[index].frames[j].bpp = 4;
							CorrectSTBILoadMemoryLayout(buf, w, h);
						}
					}
					else if (type == 1) {
						int num_anim_frames = textures_json[i]["animation_frames"].int_value();
						tilemap_renderer.feature_atlases[index].num_anim_frames = num_anim_frames;
						tilemap_renderer.feature_atlases[index].num_subtile_variants = textures_json[i]["subtile_variants"].int_value();

						tilemap_renderer.feature_atlases[index].frames = (Bitmap*)calloc(num_anim_frames, sizeof(Bitmap));

						for (int j = 0; j < num_anim_frames; j++) {
							snprintf(buffer, 256, "assets/%s_%d.bmp", file_name.c_str(), j);
							uchar* buf = stbi_load(buffer, &w, &h, &n, 4);
							tilemap_renderer.feature_atlases[index].frames[j].buffer = buf;
							tilemap_renderer.feature_atlases[index].frames[j].width = w;
							tilemap_renderer.feature_atlases[index].frames[j].height = h;
							tilemap_renderer.feature_atlases[index].frames[j].bpp = 4;
							CorrectSTBILoadMemoryLayout(buf, w, h);
						}
					}
					else if (type == 2) {
						int num_anim_frames = textures_json[i]["animation_frames"].int_value();
						tilemap_renderer.structure_atlases[index].num_anim_frames = num_anim_frames;
						tilemap_renderer.structure_atlases[index].num_subtile_variants = textures_json[i]["subtile_variants"].int_value();

						tilemap_renderer.structure_atlases[index].frames = (Bitmap*)calloc(num_anim_frames, sizeof(Bitmap));

						for (int j = 0; j < num_anim_frames; j++) {
							snprintf(buffer, 256, "assets/%s_%d.bmp", file_name.c_str(), j);
							uchar* buf = stbi_load(buffer, &w, &h, &n, 4);
							tilemap_renderer.structure_atlases[index].frames[j].buffer = buf;
							tilemap_renderer.structure_atlases[index].frames[j].width = w;
							tilemap_renderer.structure_atlases[index].frames[j].height = h;
							tilemap_renderer.structure_atlases[index].frames[j].bpp = 4;
							CorrectSTBILoadMemoryLayout(buf, w, h);
						}
					}
					else if (type == 3) {
						int num_anim_frames = textures_json[i]["animation_frames"].int_value();
						tilemap_renderer.unit_atlases[index].num_anim_frames = num_anim_frames;
						tilemap_renderer.unit_atlases[index].num_subtile_variants = textures_json[i]["subtile_variants"].int_value();

						tilemap_renderer.unit_atlases[index].frames = (Bitmap*)calloc(num_anim_frames, sizeof(Bitmap));

						for (int j = 0; j < num_anim_frames; j++) {
							snprintf(buffer, 256, "assets/%s_%d.bmp", file_name.c_str(), j);
							uchar* buf = stbi_load(buffer, &w, &h, &n, 4);
							tilemap_renderer.unit_atlases[index].frames[j].buffer = buf;
							tilemap_renderer.unit_atlases[index].frames[j].width = w;
							tilemap_renderer.unit_atlases[index].frames[j].height = h;
							tilemap_renderer.unit_atlases[index].frames[j].bpp = 4;
							CorrectSTBILoadMemoryLayout(buf, w, h);
						}
					}
					else {
						// error case
						DebugPrint((char*)"Yo dawg something broke in the json texture atlas loading for the tile terrain, features, and structures\n");
					}
				}
			}

			// json testing
			{
				debug_ReadFileResult texture_defines = debug_ReadFile((char*)"assets/textures.json");
				json11::Json textures_json = json11::Json::parse((char*)texture_defines.data, json_err_str);

				int num_tex_atlases = 0;
				while (textures_json[num_tex_atlases].is_object()) {
					num_tex_atlases += 1;
				}

				tilemap_renderer.num_tex_atlases_json = num_tex_atlases;
				tilemap_renderer.tex_atlases_json = (TextureAtlas*)calloc(num_tex_atlases, sizeof(TextureAtlas));

				char buffer[256];
				int w = 0, h = 0, n = 0;
				for (int i = 0; i < num_tex_atlases; i++) {
					std::string file_name = textures_json[i]["name"].string_value();
					tilemap_renderer.tex_atlases_json[i].num_anim_frames = textures_json[i]["animation_frames"].int_value();
					tilemap_renderer.tex_atlases_json[i].num_subtile_variants = textures_json[i]["subtile_variants"].int_value();

					tilemap_renderer.tex_atlases_json[i].frames = (Bitmap*)calloc(tilemap_renderer.tex_atlases_json[i].num_anim_frames, sizeof(Bitmap));

					for (int j = 0; j < tilemap_renderer.tex_atlases_json[i].num_anim_frames; j++) {
						snprintf(buffer, 256, "assets/%s_%d.bmp", file_name.c_str(), j);
						uchar* buf = stbi_load(buffer, &w, &h, &n, 4);
						tilemap_renderer.tex_atlases_json[i].frames[j].buffer = buf;
						tilemap_renderer.tex_atlases_json[i].frames[j].width = w;
						tilemap_renderer.tex_atlases_json[i].frames[j].height = h;
						tilemap_renderer.tex_atlases_json[i].frames[j].bpp = 4;
						CorrectSTBILoadMemoryLayout(buf, w, h);
					}
				}
			}



			// Generate storage pattern for texture atlases
			{
				FILE* file;
				errno_t err;
				char buffer[256];
				int w = 0, h = 0, n = 0;

				// figure out how many texture atlases we have
				for (int i = 0; i < INT_MAX; i++) {
					snprintf(buffer, 256, "assets/ta_%d_0.bmp", i);
					if ((err = fopen_s(&file, buffer, "r"))) {
						// failed to find ta_i so we are done
						tilemap_renderer.num_tex_atlases = i;
						break;
					}
					else {
						// successfully found ta_i now we count the anim frames
						fclose(file);
					}
				}

				// allocate the texture atlases
				tilemap_renderer.tex_atlases = (TextureAtlas*)calloc(tilemap_renderer.num_tex_atlases, sizeof(TextureAtlas));
				TextureAtlas* tas = tilemap_renderer.tex_atlases;

				// figure out how many animation frames per atlas
				for (int i = 0; i < tilemap_renderer.num_tex_atlases; i++) {
					for (int j = 0; j < INT_MAX; j++) {
						snprintf(buffer, 256, "assets/ta_%d_%d.bmp", i, j);
						if ((err = fopen_s(&file, buffer, "r"))) {
							// failed to open ta_i_anim_j so there's no more frames
							tas[i].num_anim_frames = j;
							break;
						}
						else {
							// successfully found ta_i_anim_j keep counting
							continue;
						}
					}
				}

				// load the frames
				for (int i = 0; i < tilemap_renderer.num_tex_atlases; i++) {
					tas[i].frames = (Bitmap*)calloc(tas[i].num_anim_frames, sizeof(Bitmap));
					for (int j = 0; j < tas[i].num_anim_frames; j++) {
						snprintf(buffer, 256, "assets/ta_%d_%d.bmp", i, j);
						uchar* buf = stbi_load(buffer, &w, &h, &n, 4);
						tas[i].frames[j].buffer = buf;
						tas[i].frames[j].width = w;
						tas[i].frames[j].height = h;
						tas[i].frames[j].bpp = 4;
						CorrectSTBILoadMemoryLayout(buf, w, h);
					}
				}
			}

			Bitmap editing_tm_viewport = { NULL, 0, 0, 0 };
			editing_tm_viewport.width = 64;
			editing_tm_viewport.height = 64;
			editing_tm_viewport.bpp = 4;
			editing_tm_viewport.buffer = (uchar*)calloc(editing_tm_viewport.bpp * editing_tm_viewport.width * editing_tm_viewport.height, sizeof(uchar));
			TilemapRenderer editing_tilemap_renderer(32, 32, 1, 0, 0, editing_tm_viewport.width, editing_tm_viewport.height, 4, 0.25f, editing_tm_viewport);
			editing_tilemap_renderer.num_tex_atlases = tilemap_renderer.num_tex_atlases;
			editing_tilemap_renderer.tex_atlases = tilemap_renderer.tex_atlases;

			Bitmap font = { NULL, 0, 0, 0 };
			// load font
			{
				int w = 0, h = 0, n = 0;
				uchar* buf = stbi_load("assets/font.bmp", &w, &h, &n, 4);
				font.buffer = buf;
				font.width = w;
				font.height = h;
				font.bpp = 4;
			}

			const int32_t MAX_UI_IMAGE_BITMAPS = 1;
			Bitmap ui_image_bitmaps[MAX_UI_IMAGE_BITMAPS];
			{
				int w = 0, h = 0, n = 0;
				uchar* buf = stbi_load("assets/ui_image.bmp", &w, &h, &n, 4);
				ui_image_bitmaps[0].buffer = buf;
				ui_image_bitmaps[0].width = w;
				ui_image_bitmaps[0].height = h;
				ui_image_bitmaps[0].bpp = 4;
				CorrectSTBILoadMemoryLayout(buf, w, h);
			}

			GameState* gs = (GameState*)gameMemory.data;

			while (win32_running) {
				// Timing
				f32 dt = 0.0f;
				{
					LARGE_INTEGER endtimer;
					QueryPerformanceCounter(&endtimer);
					int64_t timerelapsed = endtimer.QuadPart - lasttimer.QuadPart;
					f32 msperframe = (f32)((1000.0f * (f32)timerelapsed) / (f32)perftimerfreq);
					int64_t fps = (int64_t)(perftimerfreq / timerelapsed);

					uint64_t endcyclecount = __rdtsc();
					uint64_t cycleselapsed = endcyclecount - lastcyclecount;

					char str_buffer[256];
					sprintf_s(str_buffer, "ms / frame: %f, fps: %I64d, %I64u\n", msperframe, fps, cycleselapsed);
					//OutputDebugStringA(str_buffer);

					dt = msperframe;

					lasttimer = endtimer;
					lastcyclecount = endcyclecount;
				}

				// Update Input
				{
					*newInput = {};
					for (int i = 0; i < NUM_KEYBOARD_BUTTONS; i++) {
						newInput->keyboard.buttons[i].endedDown = oldInput->keyboard.buttons[i].endedDown;
					}
					win32_UpdateInput(newInput, window);
				}

				// Update Game
				{
					GameUpdate(&gameMemory, newInput, dt);
					char str_buffer[256];
					sprintf_s(str_buffer, "mouse: %d, %d\n", newInput->mouse.x, newInput->mouse.y);
					//DebugPrint(str_buffer);
				}

				// Render Game
				{
					tilemap_renderer.animation_frame_time += dt / 1000.0f;
					if (tilemap_renderer.animation_frame_time > tilemap_renderer.animation_max_frame_time) {
						// use this line if all texture atlases have the same number of animation frames
						// tilemap_renderer.animation_frame = (tilemap_renderer.animation_frame + 1) % tilemap_renderer.animation_max_frames;
						// otherwise use this line
						tilemap_renderer.animation_frame += 1;
						tilemap_renderer.animation_frame_time = 0.0f;
					}

					// Set viewport to globalBackBuffer values so that window resizes propagate properly
					// And propogate changes to UI Rects that are anchored
					if(window_resized)
					{
						viewport.buffer = (uchar*)globalBackBuffer.memory;
						viewport.width = globalBackBuffer.width;
						viewport.height = globalBackBuffer.height;

						gs->window_width = client_width;
						gs->window_height = client_height;

						int new_width = viewport.width - gs->ui_system.rects[0].x;
						int new_height = viewport.height - gs->ui_system.rects[0].y;
						gs->ui_system.rects[0].w = new_width;
						gs->ui_system.rects[0].h = new_height;
						tilemap_renderer.ResizeViewport(new_width, new_height);

						new_width = viewport.width - gs->ui_system.rects[1].x;
						//new_height = viewport.height - gs->ui_system.rects[1].y;
						gs->ui_system.rects[1].w = new_width;
						//gs->ui_system.rects[1].h = new_height;

						new_height = viewport.height - gs->ui_system.rects[2].y;
						gs->ui_system.rects[2].h = new_height;
					}

					for (int i = 0; i < gs->ui_system.NUM_RECTS; i++) {
						UIRect r = gs->ui_system.rects[i];
						if (r.visible) {
							switch (r.type) {
								case UIRectType::BOX:
								{
									DrawUIRect(&viewport, r.x, r.y, r.w, r.h, r.line_width, { 0, 0, 0, 255 }, { 200, 205, 207, 255 });
								} break;
								case UIRectType::BUTTON:
								{
									DrawUIRect(&viewport, r.x, r.y, r.w, r.h, r.line_width, { 50, 50, 50, 255 }, { 200, 205, 207, 255 });
									DrawUIText(&viewport, r.x + 3 * r.line_width, r.y + 3 * r.line_width, r.text, r.text_len, &font);
								} break;
								case UIRectType::TEXT:
								{
									DrawUIText(&viewport, r.x, r.y, r.text, r.text_len, &font);
								} break;
								case UIRectType::IMAGE:
								{
									DrawUIRect(&viewport, r.x, r.y, r.w, r.h, r.line_width, { 50, 50, 50, 255 }, { 200, 205, 207, 255 });
									DrawSprite(&viewport, r.x + 3 * r.line_width, r.y + 3 * r.line_width, &ui_image_bitmaps[0]);
								} break;
								//case UIRectType::TILEMAP:
								//{
								//	DrawSprite(&viewport, r.x, r.y, &tilemap_renderer.tex_atlases[gs->etm_page].frames[tilemap_renderer.animation_frame % tilemap_renderer.tex_atlases[gs->etm_page].num_anim_frames]);
								//} break;
								case UIRectType::GAME:
								{
									tilemap_renderer.view_x = (int32_t)gs->x;
									tilemap_renderer.view_y = (int32_t)gs->y;
									tilemap_renderer.tile_scale = gs->s;

									tilemap_renderer.CacheTileRenderingSubtiles(&gs->tilemap);

									tilemap_renderer.DrawTilemap(&(gs->tilemap));
									DrawSprite(&viewport, r.x, r.y, &(tilemap_renderer.view_bitmap));
								} break;
								default: break;
							}
						}
					}
					
					HDC hdc = GetDC(window);
					Win32DisplayBufferInWindow(&globalBackBuffer, hdc, window_width, window_height);
					ReleaseDC(window, hdc);
				}

				// Timers
				{
					int32_t enable_game_timer_printouts = 0;
					if (enable_game_timer_printouts) {
						char buffer[256];
						uint64_t cycles = 0;
						int64_t calls = 0;
						snprintf(buffer, 256, "--- Begin Game Timer Print Outs ---\n");
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_GAME_UPDATE];
						calls = global_cycle_counter.times_called[CT_GAME_UPDATE];
						snprintf(buffer, 256, "\tGame Update Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_CACHE_SUBTILES];
						calls = global_cycle_counter.times_called[CT_CACHE_SUBTILES];
						snprintf(buffer, 256, "\tCache Subtiles Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);
						
						snprintf(buffer, 256, "--- End Game Timer Print Outs ---\n");
						DebugPrint(buffer);
					}
					
					int32_t enable_render_timer_printouts = 0;
					if (enable_render_timer_printouts) {
						char buffer[256];
						uint64_t cycles = 0;
						int64_t calls = 0;
						snprintf(buffer, 256, "--- Begin Render Timer Print Outs ---\n");
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_TM_DRAW_TILEMAP];
						calls = global_cycle_counter.times_called[CT_TM_DRAW_TILEMAP];
						snprintf(buffer, 256, "\tTM::DrawTilemap Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_TM_DRAW_SPRITE];
						calls = global_cycle_counter.times_called[CT_TM_DRAW_SPRITE];
						snprintf(buffer, 256, "\tTM::DrawSprite Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_TM_DRAW_SUBTILES];
						calls = global_cycle_counter.times_called[CT_TM_DRAW_SUBTILES];
						snprintf(buffer, 256, "\tTM::DrawSubtiles Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_UI_DRAW_RECT];
						calls = global_cycle_counter.times_called[CT_UI_DRAW_RECT];
						snprintf(buffer, 256, "\tDrawUIRect Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_DRAW_PIXEL];
						calls = global_cycle_counter.times_called[CT_DRAW_PIXEL];
						snprintf(buffer, 256, "\tDrawPixel Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_DRAW_RECT];
						calls = global_cycle_counter.times_called[CT_DRAW_RECT];
						snprintf(buffer, 256, "\tDrawRect Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_DRAW_SPRITE];
						calls = global_cycle_counter.times_called[CT_DRAW_SPRITE];
						snprintf(buffer, 256, "\tDrawSprite Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						cycles = global_cycle_counter.cycles[CT_DRAW_SPRITE_MAG];
						calls = global_cycle_counter.times_called[CT_DRAW_SPRITE_MAG];
						snprintf(buffer, 256, "\tDrawSpriteMag Cycles: %I64u, Calls: %I64d\n", cycles, calls);
						DebugPrint(buffer);

						snprintf(buffer, 256, "--- End Render Timer Print Outs ---\n");
						DebugPrint(buffer);
					}

					// clear cycle counter at end of frame
					for (int i = 0; i < CT_NUM_CYCLE_TYPES; i++) {
						global_cycle_counter.start_cycles[i] = 0;
						global_cycle_counter.cycles[i] = 0;
						global_cycle_counter.times_called[i] = 0;
					}
				}

				// Swap Input structs
				{
					Input* temp = newInput;
					newInput = oldInput;
					oldInput = temp;
				}

				window_resized = false;
			}
		}
	}

	return 0;
}
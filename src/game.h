#ifndef GAME_H
#define GAME_H

#include "universal.h"

#include "../libs/simplex.h"

#include "game_types.h"

bool LoadGameSettings(GameState* gs);

bool ValidNeighbor(Tilemap* tm, int32_t x, int32_t y, int32_t nx, int32_t ny);

void InitGameState(Memory* gameMemory);
void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt);

#endif

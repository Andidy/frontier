#ifndef GAME_H
#define GAME_H

#include "universal.h"

#include "../libs/simplex.h"

#include "game_types.h"

void InitGameState(Memory* gameMemory);
void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt);

#endif

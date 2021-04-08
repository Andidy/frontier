#ifndef GAME_H
#define GAME_H

#include "universal.h"

#include "../libs/simplex.h"

struct GameState {
	
};

void InitGameState(Memory* gameMemory, vec2 windowDimensions);
void GameUpdate(Memory* gameMemory, Input* gameInput, f32 dt);

#endif

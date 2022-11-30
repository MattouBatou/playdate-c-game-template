#pragma once

#include "pd_api.h"

typedef struct game_entity {
	float x;
	float y;
	float dx;
	float dy;
} game_entity;

typedef struct game_sprite {
	LCDSprite* sprite;
	float x;
	float y;
	float dx;
	float dy;
	float width;
	float height;
} game_sprite;
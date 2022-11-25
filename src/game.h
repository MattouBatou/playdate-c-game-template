#pragma once

#include "pd_api.h"

extern PlaydateAPI* pd;
extern const struct playdate_sys* sys;
extern const struct playdate_sound* snd;
extern const struct playdate_graphics* gfx;

typedef struct game_entity {
	float x;
	float y;
	float dx;
	float dy;
} game_entity;

int update(void* userdata);
void setPDPtr(PlaydateAPI* pd);
void setupGame(void);
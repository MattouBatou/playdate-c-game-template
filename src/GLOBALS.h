#pragma once

#include "pd_api.h"
#include <stdint.h>

// GLOBAL CONSTANTS
#define GAME_WIDTH 400
#define GAME_HEIGHT 240

// GLOBAL TYPES
typedef uint32_t uint32;

// GLOBAL game.c definitions
extern PlaydateAPI* pd;
extern const struct playdate_sys* sys;
extern const struct playdate_sound* snd;
extern const struct playdate_graphics* gfx;
extern const struct playdate_sprite* sprite;

LCDBitmap* loadImageAtPath(const char* path);
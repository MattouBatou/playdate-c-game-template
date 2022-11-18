//
//  main.c
//
//  Created by Matthew Lewis on 17/11/2022.
//  Copyright (c) 2022 Matthew Lewis. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

static int update(void* userdata);
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

const char* gameBgPath = "gameBg.png";
LCDBitmap* gameBg = NULL;

static int textWidth;
static int textHeight;
static float x;
static float y;
static float dx;
static float dy;
static char text[15] = "Game Template!";
float deltaTime = 0;

#ifdef _WINDLL
__declspec(dllexport)
#endif

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		const char* err;
		font = pd->graphics->loadFont(fontpath, &err);
		gameBg = pd->graphics->loadBitmap(gameBgPath, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		if( gameBg == NULL )
			pd->system->error("%s:%i Couldn't load image %s: %s", __FILE__, __LINE__, gameBgPath, err);

		pd->graphics->setFont(font);

		textWidth = pd->graphics->getTextWidth(font, text, strlen(text), kASCIIEncoding, 0);
		textHeight = pd->graphics->getFontHeight(font);

		x = (float)(400 - textWidth) / 2.0f;
		y = (float)(240 - textHeight) / 2.0f;
		dx = 50.0f;
		dy = 50.0f;

		// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		//pd->display->setInverted(1);
		pd->display->setRefreshRate(0);
		pd->graphics->drawBitmap(gameBg, 0, 0, kBitmapUnflipped);

		pd->system->resetElapsedTime();
		pd->graphics->setDrawMode(kDrawModeNXOR);
		pd->system->setUpdateCallback(update, pd);
	}
	\
	return 0;
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;

	// Switch back to regular drawing mode so we completely redraw all pixels in desired region.
	pd->graphics->setDrawMode(kDrawModeCopy);
	// Do clip rect here to redraw the part of the background that the text was drawn to last tick.
	pd->graphics->setScreenClipRect((int)x, (int)y, textWidth, textHeight);
	pd->graphics->drawBitmap(gameBg, 0, 0, kBitmapUnflipped);
	pd->graphics->clearClipRect();
	pd->graphics->setDrawMode(kDrawModeNXOR);

	// Framerate independent multiplier for animations.
	deltaTime = pd->system->getElapsedTime();
	pd->system->resetElapsedTime();	

	float crankDelta = pd->system->getCrankChange();

	// If crank is being moved, stop animating
	if(crankDelta != 0) 
	{
		x += (dx * crankDelta * deltaTime);
		y += (dy * crankDelta * deltaTime);
	}
	else
	{ 
		x += (dx * deltaTime);
		y += (dy * deltaTime);
	}

	// Split up the collisions to make sure the bouncing text doesn't get stuck offscreen.
	if (x < 0)
	{
		x = 0;
		dx = -dx;
	}
	else if (x > LCD_COLUMNS - textWidth)
	{
		x = (float)(LCD_COLUMNS - textWidth);
		dx = -dx;
	}

	if (y < 0)
	{
		y = 0;
		dy = -dy;
	}
	else if (y > LCD_ROWS - textHeight)
	{
		y = (float)(LCD_ROWS - textHeight);
		dy = -dy;
	}

	pd->graphics->drawText(text, strlen(text), kASCIIEncoding, (int)x, (int)y);
	
	pd->system->drawFPS(0, 0);

	return 1;
}


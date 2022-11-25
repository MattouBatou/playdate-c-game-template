#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "sound.h"

PlaydateAPI* pd;
const struct playdate_sys* sys;
const struct playdate_sound* snd;
const struct playdate_graphics* gfx;

// Display/Graphics
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

const char* gameBgPath = "images/gameBg.png";
LCDBitmap* gameBg = NULL;

int textWidth;
int textHeight;

// Sound
sound_effect bounceSFX;
sound_effect bounce2SFX;
sound_effect crankDockedSFX;
sound_effect crankUndockedSFX;

// Player
game_entity Player;
char text[15] = "Game Template!";

// Input
PDButtons currentButtonDown;
PDButtons lastButtonPushed;
PDButtons lastButtonReleased;
int crankDocked = 0;

// Update loop stuff
float deltaTime = 0;

void setPDPtr(PlaydateAPI* p) {
	pd = p;
}

LCDBitmap *loadImageAtPath(const char *path)
{
	const char *outErr = NULL;
	LCDBitmap *img = gfx->loadBitmap(path, &outErr);
	if ( outErr != NULL ) {
		pd->system->logToConsole("Error loading image at path '%s': %s", path, outErr);
	}
	return img;
}

void setupGame()
{
    sys = pd->system;
    snd = pd->sound;
    gfx = pd->graphics;

    const char* err;
    font = gfx->loadFont(fontpath, &err);
    gameBg = gfx->loadBitmap(gameBgPath, &err);
    
    if ( font == NULL )
        sys->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

    if( gameBg == NULL )
        sys->error("%s:%i Couldn't load image %s: %s", __FILE__, __LINE__, gameBgPath, err);

    gfx->setFont(font);

    textWidth = gfx->getTextWidth(font, text, strlen(text), kASCIIEncoding, 0);
    textHeight = gfx->getFontHeight(font);

    // Positions of bouncing text
    Player.x = (float)(400 - textWidth) / 2.0f;
    Player.y = (float)(240 - textHeight) / 2.0f;
    Player.dx = 50.0f;
    Player.dy = 50.0f;
    
	// Sound init
	initMidiInstruments();
	initSFXSynths();

    gfx->drawBitmap(gameBg, 0, 0, kBitmapUnflipped);
    gfx->setDrawMode(kDrawModeNXOR);

    // TODO(matt): Remember to add own sounds for docking/undocking crank
    crankDocked = sys->isCrankDocked();
    sys->setCrankSoundsDisabled(1);
};

int update(void* userdata)
{
	(void)userdata; // unused - we use a global pointer to playdate api struct.

	sys->getButtonState(&currentButtonDown, &lastButtonPushed, &lastButtonReleased);

	// Button Down with repeats
	/*if (currentButtonDown & kButtonA)
	{
		//playSFX(&Chords);
		//sys->logToConsole("current button down: %d", currentButtonDown);
	}
	else if (currentButtonDown & kButtonB)
	{
		//playSFX(&Bass);
	}
	else if (currentButtonDown & kButtonUp)
	{
		//playSFX(&Snare);
	}
	else if(lastButtonPushed)
		sys->logToConsole("last button pushed: %d", lastButtonPushed);
	else if (lastButtonReleased)
		sys->logToConsole("last button released: %d", lastButtonReleased);*/

	// Switch back to regular drawing mode so we completely redraw all pixels in desired region.
	gfx->setDrawMode(kDrawModeCopy);
	// Do clip rect here to redraw the part of the background that the text was drawn to last tick.
	gfx->setScreenClipRect((int)Player.x, (int)Player.y, textWidth, textHeight);
	gfx->drawBitmap(gameBg, 0, 0, kBitmapUnflipped);
	gfx->clearClipRect();
	gfx->setDrawMode(kDrawModeNXOR);

	// Framerate independent multiplier for animations.
	deltaTime = sys->getElapsedTime();
	sys->resetElapsedTime();	

	float crankDelta = sys->getCrankChange();

	// If crank is being moved, stop animating
	if(crankDelta != 0.f) 
	{
		Player.x += (Player.dx * crankDelta * deltaTime);
		Player.y += (Player.dy * crankDelta * deltaTime);
	}
	else
	{ 
		Player.x += (Player.dx * deltaTime);
		Player.y += (Player.dy * deltaTime);
	}

	// Split up the collisions to make sure the bouncing text doesn't get stuck offscreen.
	if (Player.x < 0.f)
	{
		Player.x = 0.f;
		Player.dx = -Player.dx;
		snd->synth->setVolume(bounceSFX.synth, bounceSFX.volumeLeft, 0.f);
		playSFX(&bounceSFX);
	}
	else if (Player.x > LCD_COLUMNS - textWidth)
	{
		Player.x = (float)(LCD_COLUMNS - textWidth);
		Player.dx = -Player.dx;
		snd->synth->setVolume(bounceSFX.synth, 0.f, bounceSFX.volumeRight);
		playSFX(&bounceSFX);
	}

	if (Player.y < 0.f)
	{
		Player.y = 0;
		Player.dy = -Player.dy;
		playSFX(&bounce2SFX);
	}
	else if (Player.y > LCD_ROWS - textHeight)
	{
		Player.y = (float)(LCD_ROWS - textHeight);
		Player.dy = -Player.dy;
		playSFX(&bounce2SFX);
	}

	// Play custom sounds when crank docks/undocks. ENSURE DEFAULT SOUNDS ARE DISABLED.
	int isCrankDocked = sys->isCrankDocked();
	if (isCrankDocked && !crankDocked)
	{
		crankDocked = 1;
		playSFX(&crankDockedSFX);
	}
	else if (!isCrankDocked && crankDocked)
	{
		crankDocked = 0;
		playSFX(&crankUndockedSFX);
	}
	
	gfx->drawText(text, strlen(text), kASCIIEncoding, (int)Player.x, (int)Player.y);
	
	sys->drawFPS(0, 0);

	return 1;
}
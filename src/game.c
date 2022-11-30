#include <stdio.h>
#include <stdlib.h>

#include "GLOBALS.h"
#include "game.h"
#include "sound.h"
#include "player.h"

PlaydateAPI* pd;
const struct playdate_sys* sys;
const struct playdate_sound* snd;
const struct playdate_graphics* gfx;
const struct playdate_sprite* sprite;

// Display/Graphics
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

const char* gameBgPath = "images/gameBg.png";
LCDSprite* gameBg = NULL;
LCDBitmap* gameBgBitmap;

int textWidth;
int textHeight;

// Sound
sound_effect bounceSFX;
sound_effect bounce2SFX;
sound_effect crankDockedSFX;
sound_effect crankUndockedSFX;

// Player
game_sprite Player;

// Test stuff
game_entity Text;
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

LCDBitmap* loadImageAtPath(const char* path)
{
	const char* outErr = NULL;
	LCDBitmap* img = gfx->loadBitmap(path, &outErr);
	if ( outErr != NULL ) {
		sys->error("Error loading image at path '%s': %s", path, outErr);
	}
	return img;
}

void setupGame()
{
    sys = pd->system;
    snd = pd->sound;
    gfx = pd->graphics;
	sprite = pd->sprite;

    const char* fonterr;
	const char* gameBgerr;
    font = gfx->loadFont(fontpath, &fonterr);
    gameBgBitmap = gfx->loadBitmap(gameBgPath, &gameBgerr);
    
    if ( font == NULL )
        sys->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, fonterr);

    if( gameBgBitmap == NULL )
        sys->error("%s:%i Couldn't load image %s: %s", __FILE__, __LINE__, gameBgPath, gameBgerr);

    gfx->setFont(font);

    textWidth = gfx->getTextWidth(font, text, strlen(text), kASCIIEncoding, 0);
    textHeight = gfx->getFontHeight(font);

    // Positions of bouncing text
	Text.x = (float)(400 - textWidth) / 2.0f;
	Text.y = (float)(240 - textHeight) / 2.0f;
    
	gameBg = sprite->newSprite();
	sprite->setImage(gameBg, gameBgBitmap, kBitmapUnflipped);
	sprite->moveTo(gameBg, (float)GAME_WIDTH / 2.f, (float)GAME_HEIGHT / 2.f);
	sprite->setDrawMode(gameBg, kDrawModeInverted);
	sprite->addSprite(gameBg);

	createPlayer();
	Player.x = (float)GAME_WIDTH / 2.f;
	Player.y = (float)GAME_HEIGHT / 2.f;
	Player.dx = 50.f;
	Player.dy = 50.f;
	PDRect bounds = sprite->getBounds(Player.sprite);
	Player.width = bounds.width;
	Player.height = bounds.height;
	sprite->moveTo(Player.sprite, Player.x, Player.y);
	sys->logToConsole("are updates enabled on player sprite? %i", sprite->updatesEnabled(Player.sprite));

	// Sound init
	initMidiInstruments();
	initSFXSynths();

    // TODO(matt): Remember to add own sounds for docking/undocking crank
    crankDocked = sys->isCrankDocked();
    sys->setCrankSoundsDisabled(1);
};

int update(void* userdata)
{
	(void)userdata; // unused - we use a global pointer to playdate api struct.

	// Framerate independent multiplier for animations.
	deltaTime = sys->getElapsedTime();
	sys->resetElapsedTime();

	// Button Down with repeats
	sys->getButtonState(&currentButtonDown, &lastButtonPushed, &lastButtonReleased);
	if (currentButtonDown & kButtonA)
	{
		//playSFX(Snare.synth);
	}
	if (currentButtonDown & kButtonB)
	{
		// do stuff
	}
	if (currentButtonDown & kButtonUp)
	{
		Player.y -= Player.dy * deltaTime;
	}
	if (currentButtonDown & kButtonDown)
	{
		Player.y += Player.dy * deltaTime;
	}
	if (currentButtonDown & kButtonLeft)
	{
		Player.x -= Player.dx * deltaTime;
		sprite->setImageFlip(Player.sprite, kBitmapFlippedX);
	}
	if (currentButtonDown & kButtonRight)
	{
		Player.x += Player.dx * deltaTime;
		sprite->setImageFlip(Player.sprite, kBitmapUnflipped);
	}
	if(lastButtonPushed)
		sys->logToConsole("last button pushed: %d", lastButtonPushed);
	if (lastButtonReleased)
		sys->logToConsole("last button released: %d", lastButtonReleased);	

	float crankDelta = sys->getCrankChange();

	// If crank is being moved, stop animating
	if(crankDelta != 0.f) 
	{
		Player.x += (Player.dx * crankDelta * deltaTime);
		Player.y += (Player.dy * crankDelta * deltaTime);
	}
	else
	{ 
		//Player.x += (Player.dx * deltaTime);
		//Player.y += (Player.dy * deltaTime);
		Player.dx = fabsf(Player.dx);
		Player.dy = fabsf(Player.dy);
	}

	// Split up the collisions to make sure the bouncing text doesn't get stuck offscreen.
	if (Player.x - (Player.width/2) < 0.f)
	{
		Player.x = Player.width / 2;
		if (crankDelta)
		{
			Player.dx = -Player.dx;
			sprite->setImageFlip(Player.sprite, kBitmapUnflipped);
		}
		snd->synth->setVolume(bounceSFX.synth, bounceSFX.volumeLeft, 0.f);
		playSFX(&bounceSFX);
	}
	else if (Player.x > LCD_COLUMNS - (Player.width/2))
	{
		Player.x = (float)(LCD_COLUMNS - (Player.width/2));
		if (crankDelta)
		{
			Player.dx = -Player.dx;
			sprite->setImageFlip(Player.sprite, kBitmapFlippedX);
		}
		snd->synth->setVolume(bounceSFX.synth, 0.f, bounceSFX.volumeRight);
		playSFX(&bounceSFX);
	}

	if (Player.y - (Player.height/2) < 0.f)
	{
		Player.y = (Player.height / 2);
		if (crankDelta)
			Player.dy = -Player.dy;
		playSFX(&bounce2SFX);
	}
	else if (Player.y > LCD_ROWS - (Player.height/2))
	{
		Player.y = (float)(LCD_ROWS - (Player.height / 2));
		if (crankDelta)
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
	
	sprite->moveTo(Player.sprite, Player.x, Player.y);
	sprite->updateAndDrawSprites();

	Text.x = (float)(400 - textWidth) / 2.0f;
	Text.y = 0;
	gfx->drawText(text, strlen(text), kASCIIEncoding, (int)Text.x, (int)Text.y);
	sys->drawFPS(0, 0);

	return 1;
}
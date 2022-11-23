//
//  main.c
//
//  Created by Matthew Lewis on 17/11/2022.
//  Copyright (c) 2022 Matthew Lewis. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

typedef uint32_t uint32;

PlaydateAPI* pd = NULL;
struct playdate_sys* sys = NULL;
struct playdate_sound* snd = NULL;
struct playdate_graphics* gfx = NULL;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

const char* gameBgPath = "gameBg.png";
LCDBitmap* gameBg = NULL;

static int textWidth;
static int textHeight;

typedef struct game_entity {
	float x;
	float y;
	float dx;
	float dy;
} game_entity;

static game_entity Player;
static char text[15] = "Game Template!";

typedef struct game_music {
	PDSynth* synth;
	PDSynthInstrument* instrument;
	ControlSignal* control;
	SoundChannel* channel;
	SequenceTrack* track;
	SoundSequence* sequence;
	float volume;
} game_music;
static game_music GameMusic;

static SoundChannel* GameSFX = NULL;

typedef struct sound_effect {
	PDSynth* synth;
	SoundWaveform waveForm;
	float attackTime;
	float decayTime;
	float sustainLevel;
	float releaseTime;
	float volumeLeft;
	float volumeRight;
	float noteFreq;
	float noteVel;
	float noteLength;
	uint32 noteDelay;
} sound_effect;

static sound_effect bounce;
static sound_effect bounce2;

float deltaTime = 0;

static int update(void* userdata);

static void setupSFX(SoundChannel* fxChannel, sound_effect* fx, float volume)
{
	// 1.	New channel
	if(fxChannel == NULL)
		fxChannel = snd->channel->newChannel();
	//snd->addChannel(fxChannel);
	// 2.	New synth
	fx->synth = snd->synth->newSynth();
	// 3.	Add synth to channel as a source
	snd->channel->addSource(fxChannel, (SoundSource*)fx->synth);
	// 4.	Set channel volume
	snd->channel->setVolume(fxChannel, volume);
	// 5.	Adjust Synth Settings
	snd->synth->setWaveform(fx->synth, fx->waveForm);
	snd->synth->setAttackTime(fx->synth, fx->attackTime);
	snd->synth->setDecayTime(fx->synth, fx->decayTime);
	snd->synth->setSustainLevel(fx->synth, fx->sustainLevel);
	snd->synth->setReleaseTime(fx->synth, fx->releaseTime);
	snd->synth->setVolume(fx->synth, fx->volumeLeft, fx->volumeRight);
}

static void playSFX(sound_effect* fx)
{
	snd->synth->playNote(fx->synth, 
		fx->noteFreq, fx->noteVel, fx->noteLength, fx->noteDelay);
}

static void setupRndMusic()
{
	// 6.	Add events to control signal.
	// 7.	Setup Channel.
	// 8.	Add instrument to channel->addSource()
	// 9.	Add control signal to track
	// 10.	Add instrument to track.
	// 11.	Add notes to track. (Could be done using MIDI files?)
	// 12.	Setup Sequence.
	// 13.	Add track to sequence
	// 14.	Play sequence.

	// 1.	Setup synth
	GameMusic.volume = 0.5;
	GameMusic.synth = snd->synth->newSynth();
	snd->synth->setWaveform(GameMusic.synth, kWaveformSquare);
	snd->synth->setVolume(GameMusic.synth, GameMusic.volume, GameMusic.volume);
	snd->synth->setAttackTime(GameMusic.synth, 0.4303f);
	snd->synth->setDecayTime(GameMusic.synth, 0.37f);
	snd->synth->setSustainLevel(GameMusic.synth, 0.4f);
	snd->synth->setReleaseTime(GameMusic.synth, 0.257f);

	// 2	Setup Instrument.
	GameMusic.instrument = snd->instrument->newInstrument();
	
	// 3	Add synth to Instrument voice.
	snd->instrument->addVoice(GameMusic.instrument, GameMusic.synth, 0, 127, 0);

	// 4	Setup Track
	GameMusic.track = snd->track->newTrack();

	// 5	Setup Control Signal.
	GameMusic.control = snd->track->getSignalForController(GameMusic.track, 0, 1);

}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		pd = playdate;
		sys = playdate->system;
		snd = playdate->sound;
		gfx = playdate->graphics;

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

		// SOUND
		//setupRndMusic();
		bounce.waveForm = kWaveformSquare;
		bounce.attackTime = 0.f;
		bounce.decayTime = 0.f;
		bounce.sustainLevel = 1.f;
		bounce.releaseTime = 0.1f;
		bounce.volumeLeft = .05f;
		bounce.volumeRight = .05f;
		bounce.noteFreq = 739.99f; // F#5/Gb5
		bounce.noteVel = 1.f;
		bounce.noteLength = 0.1f;
		bounce.noteDelay = 0;
		setupSFX(GameSFX, &bounce, 1.f);

		bounce2.waveForm = kWaveformSquare;
		bounce2.attackTime = 0.f;
		bounce2.decayTime = 0.f;
		bounce2.sustainLevel = 1.f;
		bounce2.releaseTime = 0.1f;
		bounce2.volumeLeft = .05f;
		bounce2.volumeRight = .05f;
		bounce2.noteFreq = 659.25f; // E5
		bounce2.noteVel = 1.f;
		bounce2.noteLength = 0.1f;
		bounce2.noteDelay = 0;
		setupSFX(GameSFX, &bounce2, 1.f);

		// NOTE: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->display->setRefreshRate(0);
		gfx->drawBitmap(gameBg, 0, 0, kBitmapUnflipped);

		sys->resetElapsedTime();
		gfx->setDrawMode(kDrawModeNXOR);
		sys->setUpdateCallback(update, pd);
	}
	
	return 0;
}

static int update(void* userdata)
{
	(void)userdata; // unused - we use a global pointer to playdate api struct.

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
	if(crankDelta != 0) 
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
	if (Player.x < 0)
	{
		Player.x = 0;
		Player.dx = -Player.dx;
		playSFX(&bounce);
	}
	else if (Player.x > LCD_COLUMNS - textWidth)
	{
		Player.x = (float)(LCD_COLUMNS - textWidth);
		Player.dx = -Player.dx;
		playSFX(&bounce);
	}

	if (Player.y < 0)
	{
		Player.y = 0;
		Player.dy = -Player.dy;
		playSFX(&bounce2);
	}
	else if (Player.y > LCD_ROWS - textHeight)
	{
		Player.y = (float)(LCD_ROWS - textHeight);
		Player.dy = -Player.dy;
		playSFX(&bounce2);
	}

	gfx->drawText(text, strlen(text), kASCIIEncoding, (int)Player.x, (int)Player.y);
	
	sys->drawFPS(0, 0);

	return 1;
}


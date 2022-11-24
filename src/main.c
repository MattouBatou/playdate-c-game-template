//
//  main.c
//
//  Created by Matthew Lewis on 17/11/2022.
//  Copyright (c) 2022 Matthew Lewis. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

// Instrument aliases
// NOTE(matt): 0 indexed values skipping 0 and 1 due to those always being empty tracks in the exported .mid file.
#define CHORDS		2
#define BASS		3
#define SNARE		4

typedef uint32_t uint32;

typedef struct game_entity {
	float x;
	float y;
	float dx;
	float dy;
} game_entity;

typedef struct game_music {
	SoundSequence* sequence;
	SoundChannel* channel;
	float volume;
	int musicEnabled;
} game_music;

typedef struct midi_synth {
	PDSynth* synth;
	SoundWaveform waveForm;
	float attackTime;
	float decayTime;
	float sustainLevel;
	float releaseTime;
	float volumeLeft;
	float volumeRight;
} midi_synth;

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

PlaydateAPI* pd = NULL;
const struct playdate_sys* sys = NULL;
const struct playdate_sound* snd = NULL;
const struct playdate_graphics* gfx = NULL;

// Display/Graphics
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

const char* gameBgPath = "images/gameBg.png";
LCDBitmap* gameBg = NULL;

static int textWidth;
static int textHeight;

// Player
static game_entity Player;
static char text[15] = "Game Template!";


// Sound
static game_music GameMusic;
static SoundChannel* GameSFX = NULL;

static midi_synth Chords;
static midi_synth Bass;
static midi_synth Snare;

static sound_effect bounceSFX;
static sound_effect bounce2SFX;
static sound_effect crankDockedSFX;
static sound_effect crankUndockedSFX;

// Input
static PDButtons currentButtonDown;
static PDButtons lastButtonPushed;
static PDButtons lastButtonReleased;

// Update loop stuff
static float deltaTime = 0;
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

static void setupMIDIMusic(game_music* Music, char* midiFileName)
{
	// 1. Create new Sequence
	Music->sequence = snd->sequence->newSequence();
	// 2. Load midi file
	int midiFileSuccess = snd->sequence->loadMidiFile(Music->sequence, midiFileName); // from http://www.jsbach.net/midi/midi_goldbergvariations.html
	
	if (!midiFileSuccess)
	{
		// TODO: Throw error?
	}
	// 3. Get track count.
	int trackCount = snd->sequence->getTrackCount(Music->sequence);

	Music->channel = snd->channel->newChannel();

	for(int trackIndex = 0;
		trackIndex < trackCount;
		++trackIndex)
	{
		PDSynthInstrument* inst = snd->instrument->newInstrument();
		snd->instrument->setVolume(inst, Music->volume, Music->volume);
		
		// NOTE (matt): Do we need 1 channel per instrument?
		snd->channel->addSource(Music->channel, (SoundSource*)inst);

		SequenceTrack* track = snd->sequence->getTrackAtIndex(Music->sequence, trackIndex);
		PDSynthInstrument* testInst = snd->track->getInstrument(track);
		snd->track->setInstrument(track, inst);
		int controlSigCount = snd->track->getControlSignalCount(track);
		
		// NOTE(matt): For now, no polyphonic instruments. Only 1 instrument per track.
		// setup synth instrument for track
		midi_synth selectedSynthVoice;
		selectedSynthVoice.waveForm = kWaveformSquare;
		selectedSynthVoice.attackTime = .05f;
		selectedSynthVoice.decayTime = .05f;
		selectedSynthVoice.sustainLevel = .1f;
		selectedSynthVoice.releaseTime = 0.1f;
		selectedSynthVoice.volumeLeft = .05f;
		selectedSynthVoice.volumeRight = .05f;

		if (trackIndex == CHORDS)
			selectedSynthVoice = Chords;
		else if (trackIndex == BASS)
			selectedSynthVoice = Bass;
		else if (trackIndex == SNARE)
			selectedSynthVoice = Snare;

		PDSynth* synth = snd->synth->newSynth();
		snd->synth->setWaveform(synth, selectedSynthVoice.waveForm);
		snd->synth->setAttackTime(synth, selectedSynthVoice.attackTime);
		snd->synth->setDecayTime(synth, selectedSynthVoice.decayTime);
		snd->synth->setSustainLevel(synth, selectedSynthVoice.sustainLevel);
		snd->synth->setReleaseTime(synth, selectedSynthVoice.releaseTime);

		// add voice to inst
		snd->instrument->addVoice(inst, synth, 0, 127, 0.0f);
	}
}

static int crankDocked = 0;

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
		//	Music
		Chords.waveForm = kWaveformSquare;
		Chords.attackTime = 0.f;
		Chords.decayTime = 0.f;
		Chords.sustainLevel = 1.f;
		Chords.releaseTime = 0.1f;
		Chords.volumeLeft = .05f;
		Chords.volumeRight = .05f;

		Bass.waveForm = kWaveformSawtooth;
		Bass.attackTime = 0.f;
		Bass.decayTime = 0.f;
		Bass.sustainLevel = 1.f;
		Bass.releaseTime = 0.1f;
		Bass.volumeLeft = .05f;
		Bass.volumeRight = .05f;

		Snare.waveForm = kWaveformNoise;
		Snare.attackTime = 0.f;
		Snare.decayTime = 0.f;
		Snare.sustainLevel = 1.f;
		Snare.releaseTime = 0.1f;
		Snare.volumeLeft = .05f;
		Snare.volumeRight = .05f;
		
		GameMusic.volume = .025f;
		GameMusic.musicEnabled = 1;
		//setupMIDIMusic(&GameMusic, "sound/final_countdown.mid");
		setupMIDIMusic(&GameMusic, "sound/8BitTemplateTest.mid");
		snd->sequence->play(GameMusic.sequence, NULL, pd);

		//	SFX
		bounceSFX.waveForm = kWaveformSquare;
		bounceSFX.attackTime = 0.f;
		bounceSFX.decayTime = 0.f;
		bounceSFX.sustainLevel = 1.f;
		bounceSFX.releaseTime = 0.1f;
		bounceSFX.volumeLeft = .05f;
		bounceSFX.volumeRight = .05f;
		bounceSFX.noteFreq = 739.99f; // F#5/Gb5
		bounceSFX.noteVel = 1.0;
		bounceSFX.noteLength = 0.1f;
		bounceSFX.noteDelay = 0;
		setupSFX(GameSFX, &bounceSFX, 1.f);


		bounce2SFX.waveForm = kWaveformSquare;
		bounce2SFX.attackTime = 0.f;
		bounce2SFX.decayTime = 0.f;
		bounce2SFX.sustainLevel = 1.f;
		bounce2SFX.releaseTime = 0.1f;
		bounce2SFX.volumeLeft = .05f;
		bounce2SFX.volumeRight = .05f;
		bounce2SFX.noteFreq = 659.25f; // E5
		bounce2SFX.noteVel = 1.f;
		bounce2SFX.noteLength = 0.1f;
		bounce2SFX.noteDelay = 0;
		setupSFX(GameSFX, &bounce2SFX, 1.f);

		crankDockedSFX.waveForm = kWaveformSquare;
		crankDockedSFX.attackTime = 0.f;
		crankDockedSFX.decayTime = 0.f;
		crankDockedSFX.sustainLevel = 1.f;
		crankDockedSFX.releaseTime = 0.1f;
		crankDockedSFX.volumeLeft = .0f;
		crankDockedSFX.volumeRight = .05f;
		crankDockedSFX.noteFreq = 32.70f; // C1
		crankDockedSFX.noteVel = 1.0;
		crankDockedSFX.noteLength = 0.2f;
		crankDockedSFX.noteDelay = 0;
		setupSFX(GameSFX, &crankDockedSFX, 1.f);

		crankUndockedSFX.waveForm = kWaveformSquare;
		crankUndockedSFX.attackTime = 0.f;
		crankUndockedSFX.decayTime = 0.f;
		crankUndockedSFX.sustainLevel = 1.f;
		crankUndockedSFX.releaseTime = 0.1f;
		crankUndockedSFX.volumeLeft = .0f;
		crankUndockedSFX.volumeRight = .05f;
		crankUndockedSFX.noteFreq = 36.71f; // D1
		crankUndockedSFX.noteVel = 1.0;
		crankUndockedSFX.noteLength = 0.2f;
		crankUndockedSFX.noteDelay = 0;
		setupSFX(GameSFX, &crankUndockedSFX, 1.f);

		// NOTE: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->display->setRefreshRate(0);
		
		gfx->drawBitmap(gameBg, 0, 0, kBitmapUnflipped);
		gfx->setDrawMode(kDrawModeNXOR);

		// TODO(matt): Remember to add own sounds for docking/undocking crank
		crankDocked = sys->isCrankDocked();
		sys->setCrankSoundsDisabled(1);
		sys->setUpdateCallback(update, pd);
	}
	
	return 0;
}

static int update(void* userdata)
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


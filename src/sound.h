#pragma once

#include "types.h"
#include "pd_api.h"

// Instrument aliases for .mid files (My own convention).
// NOTE(matt): 0 indexed values skipping 0 and 1 due to those always being empty tracks in the exported .mid file.
#define CHORDS		2
#define BASS		3
#define SNARE		4

typedef struct game_music {
	SoundSequence* sequence;
	SoundChannel* channel;
	float volume;
	int musicEnabled;
	int loop;
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

extern game_music GameMusic;

extern sound_effect bounceSFX;
extern sound_effect bounce2SFX;
extern sound_effect crankDockedSFX;
extern sound_effect crankUndockedSFX;

void setupSFX(SoundChannel* fxChannel, sound_effect* fx, float volume);
void playSFX(sound_effect* fx);
void setupMIDIMusic(game_music* Music, char* midiFileName);
void initMidiInstruments();
void initSFXSynths();

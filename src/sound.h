#pragma once

#include "GLOBALS.h"
#include "pd_api.h"

// Instrument aliases for .mid files.
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
	int tempo;
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

extern game_music Music1;
extern game_music Music2;

midi_synth Chords;
midi_synth Bass;
midi_synth Snare;

extern sound_effect bounceSFX;
extern sound_effect bounce2SFX;
extern sound_effect crankDockedSFX;
extern sound_effect crankUndockedSFX;

void setupSFX(SoundChannel* fxChannel, sound_effect* fx, float volume);
void playSFX(sound_effect* fx);
void setupMIDIMusic(game_music* Music, char* midiFileName);
void initMidiInstruments();
void initSFXSynths();
void swapMidiFileAcb(SoundSequence* seq, void* userdata);
void swapMidiFileBcb(SoundSequence* seq, void* userdata);

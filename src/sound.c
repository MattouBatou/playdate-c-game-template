#include "GLOBALS.h"
#include "sound.h"

static game_music Music1;
static game_music Music2;

SoundChannel* GameSFX = NULL;

midi_synth Chords;
midi_synth Bass;
midi_synth Snare;

sound_effect bounceSFX;
sound_effect bounce2SFX;
sound_effect crankDockedSFX;
sound_effect crankUndockedSFX;

void swapMidiFileAcb(SoundSequence* seq, void* userdata) {
	snd->sequence->setTime(Music2.sequence, 0);
	snd->sequence->play(Music2.sequence, swapMidiFileBcb, pd);
}

void swapMidiFileBcb(SoundSequence* seq, void* userdata) {
	snd->sequence->setTime(Music1.sequence, 0);
	snd->sequence->play(Music1.sequence, swapMidiFileAcb, pd);
}

void initMidiInstruments() 
{
	sys->logToConsole("GAME_WIDTH: %i", GAME_WIDTH);
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

	Music1.volume = .025f;
	Music1.musicEnabled = 1;
	Music1.loop = 1;
	Music1.tempo = 0;

	Music2.volume = .025f;
	Music2.musicEnabled = 1;
	Music2.loop = 1;
	Music2.tempo = 0;

	setupMIDIMusic(&Music1, "sound/8BitTemplateTest.mid");
	setupMIDIMusic(&Music2, "sound/final_countdown.mid");


	snd->sequence->play(Music1.sequence, NULL, NULL);
};

void initSFXSynths() 
{
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
};

void setupSFX(SoundChannel* fxChannel, sound_effect* fx, float volume)
{
	// 1.	New channel
	if (fxChannel == NULL)
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

void playSFX(sound_effect* fx)
{
	snd->synth->playNote(fx->synth,
		fx->noteFreq, fx->noteVel, fx->noteLength, fx->noteDelay);
}

void setupMIDIMusic(game_music* Music, char* midiFileName)
{
	// 1. Create new Sequence
	Music->sequence = snd->sequence->newSequence();
	// 2. Load midi file
	int midiFileSuccess = snd->sequence->loadMidiFile(Music->sequence, midiFileName);

	if (!midiFileSuccess) {
		// TODO: Throw error?
		sys->error("could not load midi file!");
	}

	if (Music->tempo > 0)
	{
		snd->sequence->setTempo(Music->sequence, Music->tempo);
	}

	// 3. Get track count.
	int trackCount = snd->sequence->getTrackCount(Music->sequence);

	Music->channel = snd->channel->newChannel();

	for (int trackIndex = 0;
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

	if (Music->loop) 
	{
		snd->sequence->setLoops(Music->sequence, 0, snd->sequence->getLength(Music->sequence), 0);
	}	
}
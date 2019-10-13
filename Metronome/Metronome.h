/*
 *  Metronome.h
 *
 *	Special thanks to Vincent Falco, a.k.a "The Vinn" on the JUCE Forums for sharing his Metronome clocking code
 *	Which has been adapted and used here.
 * 
 *  Created by Jordan Hochenbaum on 2/4/11.
 *  Copyright 2011 FlipMu. All rights reserved.
 *
 */

#ifndef _METRONOME_
#define _METRONOME_

#include <memory>
#include "../../StaticJuce/JuceLibraryCode/JuceHeader.h"
#include "MetronomeBeatCountListener.h"

class Metronome : public juce::AudioSource
{
public:
    using TimeSignature = std::pair<int, int>;
    
public:
	Metronome(bool* countedIn);
	~Metronome();
	
	void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate);
    void releaseResources();
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
    void getNextAudioBlock(float *const *buffer, int numChannels, int size);
	
	void setBPM(float beatsPerMinute) noexcept;
    float getBPM() const noexcept;
	void setMetroRunning(bool runMetro);
	void setMetroPlaying(bool playMetro);
	int getBeatCount();
	void addBeatListener(MetronomeBeatCountListener* mbl);
	void removeBeatListener(MetronomeBeatCountListener* mbl);
    bool isCountedIn() noexcept;
    void resetCountIn() noexcept;
    void setTimeSignature(const TimeSignature &timeSignature) noexcept;
    TimeSignature timeSignature() const noexcept;
    bool isPlaying() noexcept { return m_shouldPlay; }

private:
	void setupBar(int startingBeat, const TimeSignature &signature);
	void advanceClock(int numSamples);
	void callBeatListeners();
	float m_sampleRate, m_clockPhase;
	int m_currentBeat;
    TimeSignature m_timeSignature;
    float m_bpm;
    int m_beatCount;
	bool m_shouldRun, m_shouldPlay;
    bool* m_countIn = nullptr;
    juce::WavAudioFormat*	m_wavAudioFormat = nullptr;//we will add a waveAudioFormat to our audioFormatManager
	juce::AudioFormatManager m_audioFormatManager;//manages a list of formats we want to read (currently only .wav files)
	
    std::unique_ptr<juce::AudioFormatReader> m_metroHighReader;//reader to load metronome .wav #1
	std::unique_ptr<juce::AudioFormatReader> m_metroLowReader;//reader to load metronome .wav #2
	
	juce::Synthesiser m_samplerSynth;//the synth to play the sound

	juce::ListenerList<MetronomeBeatCountListener> m_beatListeners;
};

#endif //_METRONOME_

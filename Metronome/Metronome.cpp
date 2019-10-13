/*
 *  Metronome.cpp
 *  Nuance
 *
 *  Created by Jordan Hochenbaum on 2/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Metronome.h"

namespace {
    const int QUARTER_NOTE_RATIO = 4;
    const int SECONDS_PER_MININUTE = 60;
}

Metronome::Metronome(bool* countedIn) : m_clockPhase(0),
                                        m_currentBeat(0),
                                        m_timeSignature({4, 4}),
                                        m_bpm(120),
                                        m_shouldRun(false),
                                        m_shouldPlay(false),
                                        m_countIn(countedIn)
{
	m_audioFormatManager.registerFormat(m_wavAudioFormat = new WavAudioFormat(), true);
	
    //First we must load out metronome .wav files into a reader
    juce::File resourceRoot = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile);
    const juce::String metroHighPath = resourceRoot.getChildFile("./MetronomeHigh.wav").getFullPathName();//"~/Sound/metronomeHigh.wav";
    const juce::String metroLowPath = resourceRoot.getChildFile("./MetronomeLow.wav").getFullPathName();
    m_metroHighReader = std::unique_ptr<juce::AudioFormatReader>(m_audioFormatManager.createReaderFor(File(metroHighPath)));
    m_metroLowReader = std::unique_ptr<juce::AudioFormatReader>(m_audioFormatManager.createReaderFor(File(metroLowPath)));

	//Define the midi note ranges each sound will be assigned to...
	BigInteger highNotes;
	highNotes.setRange(0, 1, true);
	BigInteger lowNotes;
	lowNotes.setRange(2, 3, true);
	
	//then we must create SamplerSounds from the readers, and add them to our synth
	m_samplerSynth.addVoice(new SamplerVoice()); //this voice will be used to play the sound-- only one sound will play at a time so we only add one voice (monophonic)
	m_samplerSynth.addSound(new SamplerSound("metroHigh", *m_metroHighReader, highNotes, 0, 0, 0, 1.0));
	m_samplerSynth.addSound(new SamplerSound("metroLow", *m_metroLowReader, lowNotes, 2, 0, 0, 1.0));
	m_samplerSynth.setNoteStealingEnabled(true); //must turn note stealing off
    setupBar(0, {4, 4});
}

Metronome::~Metronome() {	
	m_samplerSynth.clearSounds();
	m_samplerSynth.clearVoices();
}

void Metronome::prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) {
	m_samplerSynth.setCurrentPlaybackSampleRate(sampleRate);
	m_sampleRate = sampleRate;
}

void Metronome::releaseResources() {
}

void Metronome::getNextAudioBlock(float *const *buffer, int numChannels, int size) {
    AudioSampleBuffer audioBuffer{buffer, numChannels, size};
    AudioSourceChannelInfo info{&audioBuffer, 0, size};
    getNextAudioBlock(info);
}

void Metronome::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill){
	
	int const numSamples = bufferToFill.numSamples;
    bufferToFill.clearActiveBufferRegion();
	
	MidiBuffer midi;
	
    if (m_shouldRun) {
        const double beatRatio = static_cast<double>(QUARTER_NOTE_RATIO) / m_timeSignature.second;
        double const samplesPerBeat = m_sampleRate * SECONDS_PER_MININUTE * beatRatio / m_bpm;
		
		// Adjust phase so the beat is on or after the beginning of the output
		double beat = 0;
		if (m_clockPhase > 0)
			beat = 1 - m_clockPhase;
		else
			beat = 0 - m_clockPhase;
		
		// Set notes in midi buffer
        for (;;beat += 1) {
			// Calc beat pos
			int pos = static_cast <int> (beat * samplesPerBeat);
			if (pos < numSamples) {
				if(m_shouldPlay) {
					if (m_currentBeat == 0) {
						m_samplerSynth.noteOn(1, 60, 1.0);
						midi.addEvent(MidiMessage::noteOn (1, 0, 1.f), pos);
					} else {
						midi.addEvent(MidiMessage::noteOn (1, 2, 1.f), pos);
					}
				}
                
                //if we aren't yet counted in, check to see if enough beats have past to set counted in
                if(*m_countIn == false) {
                    if (m_currentBeat == 3) {
                        std::cout << "setting count in" << std::endl;
                        *m_countIn = true;
                    }
                }
                
                callBeatListeners();
                
				m_currentBeat++;
				m_currentBeat =  m_currentBeat % m_timeSignature.first;
            } else {
				break;
			}
		}
        advanceClock(numSamples);
	}
	
	m_samplerSynth.renderNextBlock (*bufferToFill.buffer, midi, 0, bufferToFill.numSamples);
}

void Metronome::setupBar (int startingBeat, const TimeSignature &timeSignature) {
    *m_countIn = false;
    m_clockPhase = 0;
	m_currentBeat = startingBeat;
    setTimeSignature(timeSignature);
}

void Metronome::advanceClock (int numSamples) {
    const double beatRatio = static_cast<double>(QUARTER_NOTE_RATIO) / m_timeSignature.second;
    double const samplesPerBeat = m_sampleRate * 60 * beatRatio / m_bpm;
	
    jassert(m_clockPhase >= -.5 && m_clockPhase < .5);
	
    if (m_clockPhase < 0) {
		m_clockPhase = m_clockPhase + 1;
    }
	
    m_clockPhase = m_clockPhase + numSamples / samplesPerBeat;
	
    if (m_clockPhase >= .5) {
		m_clockPhase -= 1;
    }
	
    jassert(m_clockPhase >= -.5 && m_clockPhase < .5);
}

void Metronome::setMetroRunning(bool runMetro) {
    m_shouldRun = runMetro;
    
    if (!runMetro) {
        setupBar(0, m_timeSignature);
    }
}

void Metronome::setMetroPlaying(bool playMetro) {
	m_shouldPlay = playMetro;
}

bool Metronome::isCountedIn() noexcept {
    if (m_countIn) {
        return *m_countIn;
    }
    return false;
}

void Metronome::resetCountIn() noexcept {
    if (m_countIn) {
        *m_countIn = false;
    }
}

void Metronome::setTimeSignature(const TimeSignature &timeSignature) noexcept {
    m_timeSignature = timeSignature;
}

Metronome::TimeSignature Metronome::timeSignature() const noexcept {
    return m_timeSignature;
}

void Metronome::setBPM(float beatsPerMinute) noexcept {
	m_bpm = beatsPerMinute;
}

float Metronome::getBPM() const noexcept {
    return m_bpm;
}

int Metronome::getBeatCount() {
	return m_currentBeat;
}

void Metronome::addBeatListener(MetronomeBeatCountListener* mbl) {
	m_beatListeners.add(mbl);
}

void Metronome::removeBeatListener(MetronomeBeatCountListener* mbl) {
	m_beatListeners.remove(mbl);
}

void Metronome::callBeatListeners()
{
    m_beatListeners.call(&MetronomeBeatCountListener::metronomeCallbackMethod, m_currentBeat);
}

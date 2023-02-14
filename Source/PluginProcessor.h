/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "CircularBuffer.h"

//==============================================================================
/**
*/
class puannhiAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	puannhiAudioProcessor();
	~puannhiAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	int nextBlockReady = false;

	float target_frequency[10] = { 240,500,750,1000,1250,1500,1800,2200,3000,5000 };
	const int DivideUnit = 10;
	float drawFFT[10*10];
	const int scopeSize = 128;  
	float* scopeData = new float[scopeSize];
	int TargetFreqNum;


	const int N = 2048;
	std::complex<float>* InputArray = new std::complex<float>[N];
	std::complex<float>* OutputArray = new std::complex<float>[N];
	CircularBuffer<float> circularbuffer;
	float* previousOutputArray = new float[N];
	float* currentOutputArray = new float[N];

	double input_sample_rate = 0.0;
	int WindowTag = 1;
private:
	//==============================================================================
	juce::ScopedPointer<juce::dsp::FFT> forwardFFT;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(puannhiAudioProcessor)
};
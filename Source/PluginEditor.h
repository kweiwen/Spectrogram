/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class puannhiAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    puannhiAudioProcessorEditor (puannhiAudioProcessor&);
    ~puannhiAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

	void timerCallback() override;
	void FindIndex(int numberLeft, int numberRight, int NumberFrequecy, int* Index);
	void BandDivide(int FreqNumber, int TargetFreNum, float * OutputArray, int* FreqNumInterval, int* FreqNumArray, int N, int* Index);
	void BandInternalDivide(int NumTargetFreNum, int DivideUnit, int StartIndex, int EndIndex, float* OutputArray, float* drawArray);
	void DrawFFTSpectrumDivide(float* drawArray, float* TargetFrequency, int TargetFreNum, int N, int fs, float* OutputArray, int DivideUnit);
	void drawNextFrameOfSpectrum();
	void drawFrame(juce::Graphics& g);
	void drawCoordiante(juce::Graphics & g);

	int current_input_fftSize_index = 1;
	juce::Label l_input_channel;
	juce::ComboBox s_input_channel;
	juce::Label l_input_fftSize;
	juce::ComboBox s_input_fftSize;
	juce::StringArray current_input_channel_items;
	int current_input_channel_index = 0;
	juce::StringArray last_input_channel_items;
	juce::StringArray empty_input_channel_items;
	juce::Rectangle<int> SpectrogramArea;

	juce::Slider OutputRadio;
	juce::Label l_OutputRadio;
	float Ratio;
	int targetFreqNum;
	juce::Label l_windowfunction;
	juce::ComboBox s_windowfunction;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    puannhiAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (puannhiAudioProcessorEditor)
};

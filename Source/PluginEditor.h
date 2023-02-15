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

	void unit_test(juce::Graphics& g);
	void drawNextFrameOfSpectrum();
	void drawFrame(juce::Graphics& g);
	void drawCoordiante(juce::Graphics& g);

	juce::Label label;


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

	float mindB;
	float maxdB;
	float max;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (puannhiAudioProcessorEditor)
};

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

	juce::Rectangle<int> SpectrogramArea;

	juce::Label LwinFunc;
	juce::ComboBox CwinFunc;

	juce::Label Lratio;
	juce::Slider Sratio;
	
	juce::Label Lpeak;
	juce::Label LpeakVal;

	juce::Label LfftSize;
	juce::Label LfftSizeVal;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    puannhiAudioProcessor& audioProcessor;

	float mindB;
	float maxdB;
	float max;
	float ratio;
	float width_f;
	float height_f;
	int width_i;
	int height_i;
	float offset_x;
	float offset_y;
	float gridSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (puannhiAudioProcessorEditor)
};

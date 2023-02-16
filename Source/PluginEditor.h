/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class UI_LookAndFeel : public juce::LookAndFeel_V4
{
public:
	juce::Font getLabelFont(juce::Label&) override
	{
		return { 12.5f };
	}

	juce::Font getComboBoxFont(juce::ComboBox& /*box*/) override
	{
		return { 12.5f };
	}

	juce::Font getPopupMenuFont() override
	{
		return { 12.5f };
	}

	void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
		const bool isSeparator, const bool isActive,
		const bool isHighlighted, const bool isTicked,
		const bool hasSubMenu, const juce::String& text,
		const juce::String& shortcutKeyText,
		const juce::Drawable* icon, const juce::Colour* const textColour) override
	{
		LookAndFeel_V4::drawPopupMenuItem(g, area, 
			isSeparator, isActive, 
			isHighlighted, false, 
			hasSubMenu, text, 
			shortcutKeyText,
			icon, textColour);
		if (isTicked)
		{
			g.setColour(juce::Colours::antiquewhite);
			auto maxFontHeight = (float)area.getHeight();
			auto TopLeftPt = area.getTopLeft();
			auto x = TopLeftPt.getX();
			auto y = TopLeftPt.getY();
			auto rectangle = juce::Rectangle<float>(x, y, maxFontHeight, maxFontHeight);
			rectangle.reduce(8.0f, 8.0f);
			g.drawEllipse(rectangle, 2);
		}
	}

	void drawTickBox(juce::Graphics& g, juce::Component& component,
		float x, float y,
		float w, float h,
		bool ticked, bool isEnabled,
		bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
	{
		LookAndFeel_V4::drawTickBox(g, component,
			x, y,
			w, h,
			false, isEnabled, 
			shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

		if (ticked)
		{
			auto rect = juce::Rectangle<float>(component.getHeight(), component.getHeight());
			rect.reduce(8.0f, 8.0f);
			g.drawEllipse(rect, 2);
		}
	}
};


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
	void drawFrequency(juce::Graphics& g);
	void drawAmplitude(juce::Graphics& g);

	float inverse_x(float freq);

	juce::Rectangle<int> SpectrogramArea;

	juce::Label LwinFunc;
	juce::ComboBox CwinFunc;

	juce::Label Lratio;
	juce::Slider Sratio;
	
	juce::Label Lpeak;
	juce::Label LpeakVal;

	juce::Label LfftSize;
	juce::Label LfftSizeVal;

	juce::Label LxScale;
	juce::ToggleButton BxScale;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    puannhiAudioProcessor& audioProcessor;

	float mindB;
	float maxdB;
	float max;
	float skew;
	float ratio;
	bool isLog;
	float width_f;
	float height_f;
	int width_i;
	int height_i;
	float offset_x;
	float offset_y;
	float lineGridSize;
	float barGridSize;

	std::unique_ptr<UI_LookAndFeel> lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (puannhiAudioProcessorEditor)
};

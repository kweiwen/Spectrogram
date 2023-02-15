/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
puannhiAudioProcessorEditor::puannhiAudioProcessorEditor (puannhiAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
	targetFreqNum = sizeof(audioProcessor.target_frequency) / sizeof(audioProcessor.target_frequency[0]);
    setSize (1000, 500);
	startTimerHz(30);

	OutputRadio.setSliderStyle(juce::Slider::LinearHorizontal);
	OutputRadio.setRange(0.01, 1.0, 0.01);
	OutputRadio.setValue(0.2);
	OutputRadio.onValueChange = [this] {Ratio = OutputRadio.getValue(); };
	addAndMakeVisible(&OutputRadio);

	addAndMakeVisible(l_input_fftSize);
	l_input_fftSize.setText("FFTSize : "+juce::String(audioProcessor.N), juce::dontSendNotification);

	addAndMakeVisible(l_OutputRadio);
	l_OutputRadio.setText("Forgetting Factor:", juce::dontSendNotification);

	addAndMakeVisible(l_windowfunction);
	l_windowfunction.setText("Window Function:  ", juce::dontSendNotification);

	addAndMakeVisible(s_windowfunction);
	s_windowfunction.setTextWhenNothingSelected("Rectangular Window");
	s_windowfunction.addItem("Rectangular Window", 1);
	s_windowfunction.addItem("Hanning Window", 2);
	s_windowfunction.addItem("Hamming Window", 3);
	s_windowfunction.addItem("Blackman Window",4);
	s_windowfunction.addItem("Triangle Window", 5);
	s_windowfunction.onChange = [this] {audioProcessor.WindowTag = s_windowfunction.getSelectedId(); };

	// specific private member for analysis
	mindB = -100.0f;
	maxdB = 0.0f;
	max = -100.0f;

	addAndMakeVisible(label);
	label.setText("null", juce::dontSendNotification);
}

puannhiAudioProcessorEditor::~puannhiAudioProcessorEditor()
{
}

//==============================================================================
void puannhiAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.setOpacity(1.0f);
	g.setColour(juce::Colours::greenyellow);
	drawFrame(g);
	drawCoordiante(g);
}

void puannhiAudioProcessorEditor::resized()
{
	auto area = getLocalBounds();
	//auto audioSetupCompArea = area.removeFromLeft(area.getWidth() / 3);
	//audioSetupComp.setBounds(audioSetupCompArea);
	auto input_channel_label_area = area.removeFromTop(30);
	auto input_channel_items_area = area.removeFromTop(30);

	l_input_channel.setBounds(input_channel_label_area);
	s_input_channel.setBounds(input_channel_items_area);
	area.reduce(40, 30);
	SpectrogramArea = area;


	label.setBounds(300, 30, 150, 30);

	l_input_fftSize.setBounds(370, 30, 150, 30);

	
	OutputRadio.setBounds(150, 30, 150, 30);
	l_OutputRadio.setBounds(25, 30, 150, 30);

	l_windowfunction.setBounds(500, 30, 130, 30);
	s_windowfunction.setBounds(650, 30, 180, 30);
}


void puannhiAudioProcessorEditor::timerCallback()
{
	if (audioProcessor.nextBlockReady = true)
	{		
		drawNextFrameOfSpectrum();
		audioProcessor.nextBlockReady = false;
		repaint();
	}
}

void puannhiAudioProcessorEditor::drawNextFrameOfSpectrum()
{	
	for (int i = 0; i < audioProcessor.N; i++)
	{
		// to compensate the data outside nyquist
		auto amplitude = std::abs(audioProcessor.OutputArray[i]) * 2;
		//auto angle = std::arg(audioProcessor.frameProcessArray[i]);
		audioProcessor.currentOutputArray[i] = amplitude;
		audioProcessor.previousOutputArray[i] = Ratio * audioProcessor.currentOutputArray[i] + (1- Ratio)*audioProcessor.previousOutputArray[i];
	}

	// convert data disribution from linear into logarithm
	for (int i = 0; i < audioProcessor.scopeSize; i++)
	{
		auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)audioProcessor.scopeSize) * 0.2f);
		auto fftDataIndex = juce::jlimit(0, audioProcessor.N / 2, (int)(skewedProportionX * (float)audioProcessor.N * 0.5f));

		auto Ve = juce::Decibels::gainToDecibels(audioProcessor.previousOutputArray[fftDataIndex]);
		auto V0 = juce::Decibels::gainToDecibels((float)audioProcessor.N);

		auto level_limited = juce::jlimit(mindB, maxdB, Ve-V0);
		
		auto level = juce::jmap(level_limited, mindB, maxdB, 0.0f, 1.0f);
		
		audioProcessor.scopeData[i] = level;

		if (level_limited > max)
		{
			max = level_limited;
		}
	}

	label.setText(juce::String(max), juce::dontSendNotification);
	//audioProcessor.scopeData[0] = 1.0f;
	//audioProcessor.scopeData[1] = 1.0f;
	//audioProcessor.scopeData[2] = 1.0f;
	//audioProcessor.scopeData[3] = 1.0f;
}

void puannhiAudioProcessorEditor::drawFrame(juce::Graphics& g)
{
	auto width = SpectrogramArea.getWidth();
	auto height = SpectrogramArea.getHeight();
	float width_f = (float)width;
	float height_f = (float)height;

	float offset_x = SpectrogramArea.getX();
	float offset_y = SpectrogramArea.getY();

	auto gridSize = width / (float)audioProcessor.scopeSize;

	g.setColour(juce::Colours::grey);
	g.fillRect(offset_x, offset_y, width_f, height_f);

	// line graph
	for (int i = 1; i < audioProcessor.scopeSize; i++)
	{
		g.setColour(juce::Colours::antiquewhite);
		g.drawLine({ 
			offset_x + (float)juce::jmap(i - 1, 0, audioProcessor.scopeSize - 1, 0, width),
			offset_y + juce::jmap(audioProcessor.scopeData[i - 1], 0.0f, 1.0f, height_f, 0.0f),
			offset_x + (float)juce::jmap(i,     0, audioProcessor.scopeSize - 1, 0, width),
			offset_y + juce::jmap(audioProcessor.scopeData[i],     0.0f, 1.0f, height_f, 0.0f)
			});
	}

	// bar graph
	for (int i = 0; i < audioProcessor.scopeSize; i++)
	{
		auto val = juce::jmap(audioProcessor.scopeData[i], 0.0f, 1.0f, 0.0f, height_f);
		g.setColour(juce::Colours::greenyellow);
		g.fillRect(offset_x + i * gridSize, offset_y + (height_f - val), gridSize, height_f - (height_f - val));
	}

}

void puannhiAudioProcessorEditor::drawCoordiante(juce::Graphics & g)
{
	auto area = SpectrogramArea;
	auto width = SpectrogramArea.getWidth();
	auto height = SpectrogramArea.getHeight();
	//g.setFont(12.0f);
	//g.setColour(juce::Colours::blue);
	//auto deltaX=(float)juce::jmap(1 * 10, 0, audioProcessor.scopeSize, 0, width);
	
	//for (int i = 0; i < 10; ++i)
	//{
	//	auto x = area.getX() + (float)juce::jmap(i, 0, 10,0, width);
	//	g.setColour(juce::Colours::whitesmoke);
	//	g.drawFittedText(juce::String((audioProcessor.target_frequency[i] < 1000.0f) ? juce::String(audioProcessor.target_frequency[i]) + " Hz" : juce::String(audioProcessor.target_frequency[i] / 1000.0f) + " kHz"),
	//		juce::roundToInt(x + deltaX/4), area.getBottom() + 8, 40, 15, juce::Justification::left, 1);
	//	if (i >= 1)
	//	{
	//		g.setColour(juce::Colours::whitesmoke);
	//		g.drawVerticalLine(juce::roundToInt(area.getX() + i / 10.0f * area.getWidth()), area.getY(), area.getBottom());
	//	}
	//}
	//
	//g.setColour(juce::Colours::whitesmoke);
	//g.drawFittedText("  6 dB", area.getX() - 35, area.getY() + 2 - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("  0 dB", area.getX() - 35, juce::roundToInt(area.getY() + 0.056 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-10 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.15 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-30 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.34 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-40 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.43 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-50 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.53 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-60 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.62 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-70 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.72 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-80 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.81 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-90 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.90 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	//g.drawFittedText("-100 dB", area.getX() - 40, juce::roundToInt(area.getY() + 1.0 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);

	//g.setColour(juce::Colours::whitesmoke);
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.00 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.056 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.15 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.34 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.43 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.53 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.62 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.72 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.81 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.90 * area.getHeight()), area.getX(), area.getRight());
	//g.drawHorizontalLine(juce::roundToInt(area.getY() + 1.0 * area.getHeight()), area.getX(), area.getRight());
}


void puannhiAudioProcessorEditor::unit_test(juce::Graphics& g)
{
	auto width = SpectrogramArea.getWidth();
	auto height = SpectrogramArea.getHeight();
	float width_f = (float)width;
	float height_f = (float)height;

	float offset_x = SpectrogramArea.getX();
	float offset_y = SpectrogramArea.getY();

	g.setColour(juce::Colours::grey);
	g.fillRect(offset_x, offset_y, width_f, height_f);

	auto gridSize = width / (float)audioProcessor.scopeSize;
	for (int i = 0; i < audioProcessor.scopeSize; i++)
	{
		auto val = juce::jmap(i / (float)audioProcessor.scopeSize, 0.0f, 1.0f, 0.0f, height_f);
		g.setColour(juce::Colours::greenyellow);
		g.fillRect(offset_x + i * gridSize, offset_y + (height_f - val), gridSize, height_f - (height_f - val));
	}
}
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
    setSize (640, 480);
	startTimerHz(25);

	// specific private member for analysis
	mindB = -100.0f;
	maxdB = 0.0f;
	max = -100.0f;
	ratio = 20;

	// init ui
	Lratio.setText("Forgetting Factor", juce::dontSendNotification);
	addAndMakeVisible(Lratio);

	Sratio.setSliderStyle(juce::Slider::LinearHorizontal);
	Sratio.setRange(1, 100, 1);
	Sratio.setValue(ratio);
	Sratio.setTextValueSuffix(" %");
	Sratio.onValueChange = [this] {ratio = Sratio.getValue(); };
	addAndMakeVisible(&Sratio);

	LwinFunc.setText("Window Function", juce::dontSendNotification);
	addAndMakeVisible(LwinFunc);

	CwinFunc.setTextWhenNothingSelected("Rectangular");
	CwinFunc.addItem("Rectangular", 1);
	CwinFunc.addItem("Hanning",  2);
	CwinFunc.addItem("Hamming",  3);
	CwinFunc.addItem("Blackman", 4);
	CwinFunc.addItem("Triangle", 5);
	// should place in editor side
	CwinFunc.onChange = [this] {audioProcessor.WindowTag = CwinFunc.getSelectedId(); };
	addAndMakeVisible(CwinFunc);

	Lpeak.setText("Peak Decibel", juce::dontSendNotification);
	addAndMakeVisible(Lpeak);

	LpeakVal.setText(juce::String(mindB), juce::dontSendNotification);
	addAndMakeVisible(LpeakVal);

	LfftSize.setText("Transform Size", juce::dontSendNotification);
	addAndMakeVisible(LfftSize);

	LfftSizeVal.setText(juce::String(audioProcessor.N), juce::dontSendNotification);
	addAndMakeVisible(LfftSizeVal);
}

puannhiAudioProcessorEditor::~puannhiAudioProcessorEditor()
{
}

//==============================================================================
void puannhiAudioProcessorEditor::paint (juce::Graphics& g)
{
	drawFrame(g);
	drawCoordiante(g);
}

void puannhiAudioProcessorEditor::resized()
{
	auto area = getLocalBounds();
	area.removeFromTop(40);

	area.reduce(40, 30);
	SpectrogramArea = area;

	auto row1 = 10;
	auto row2 = 40;

	LwinFunc.setBounds(40, row1, 120, 25);
	CwinFunc.setBounds(160, row1, 250, 25);
	Lratio.setBounds(40, row2, 120, 25);
	Sratio.setBounds(160, row2, 250, 25);

	Lpeak.setBounds(420, row1, 100, 25);
	LpeakVal.setBounds(520, row1, 80, 25);
	LfftSize.setBounds(420, row2, 100, 25);
	LfftSizeVal.setBounds(520, row2, 80, 25);

	width_f = SpectrogramArea.getWidth();
	height_f = SpectrogramArea.getHeight();

	width_i = SpectrogramArea.getWidth();
	height_i = SpectrogramArea.getHeight();

	offset_x = SpectrogramArea.getX();
	offset_y = SpectrogramArea.getY();

	gridSize = width_f / (float)audioProcessor.scopeSize;
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
		audioProcessor.previousOutputArray[i] = (ratio / 100.0f) * audioProcessor.currentOutputArray[i] + (1.0f - (ratio / 100.0f)) * audioProcessor.previousOutputArray[i];
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
}

void puannhiAudioProcessorEditor::drawFrame(juce::Graphics& g)
{
	g.setColour(juce::Colours::grey);
	g.fillRect(offset_x, offset_y, width_f, height_f);

	// line graph
	for (int i = 1; i < audioProcessor.scopeSize; i++)
	{
		g.setColour(juce::Colours::antiquewhite);
		g.drawLine({ 
			offset_x + (float)juce::jmap(i - 1, 0, audioProcessor.scopeSize - 1, 0, width_i),
			offset_y + juce::jmap(audioProcessor.scopeData[i - 1], 0.0f, 1.0f, height_f, 0.0f),
			offset_x + (float)juce::jmap(i,     0, audioProcessor.scopeSize - 1, 0, width_i),
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

	LpeakVal.setText(juce::String(max), juce::dontSendNotification);
}

void puannhiAudioProcessorEditor::drawCoordiante(juce::Graphics & g)
{
	// should be driven by gui event
	g.setColour(juce::Colours::red);
	g.fillRect(offset_x, offset_y, 50.0f, 50.0f);

	g.setColour(juce::Colours::antiquewhite);
	for (int i = 0; i < 11; i++)
	{
		auto y_pos = offset_y + (i * height_f / 10);
		g.drawHorizontalLine(y_pos, offset_x, offset_x + width_f);
	}
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
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

	// change skew to 1.0f to get linear scale
	skew = 1.0f; 
	isLog = false;

	// init look and feel
	lnf.reset(new UI_LookAndFeel);

	// initialisation ui
	Lratio.setText("Forgetting Factor", juce::dontSendNotification);
	Lratio.setLookAndFeel(lnf.get());
	addAndMakeVisible(Lratio);

	Sratio.setSliderStyle(juce::Slider::LinearHorizontal);
	Sratio.setRange(1, 100, 1);
	Sratio.setValue(ratio);
	Sratio.setTextValueSuffix(" %");
	Sratio.setLookAndFeel(lnf.get());
	Sratio.onValueChange = [this] {ratio = Sratio.getValue(); };
	addAndMakeVisible(&Sratio);

	LwinFunc.setText("Window Function", juce::dontSendNotification);
	LwinFunc.setLookAndFeel(lnf.get());
	addAndMakeVisible(LwinFunc);

	CwinFunc.addItem("Rectangular", 1);
	CwinFunc.addItem("Hanning",  2);
	CwinFunc.addItem("Hamming",  3);
	CwinFunc.addItem("Blackman", 4);
	CwinFunc.addItem("Triangle", 5);
	CwinFunc.setSelectedItemIndex(0, true);
	CwinFunc.setLookAndFeel(lnf.get());
	// should place in editor side
	CwinFunc.onChange = [this] {audioProcessor.WindowTag = CwinFunc.getSelectedId(); };
	addAndMakeVisible(CwinFunc);

	Lpeak.setText("Peak Decibel", juce::dontSendNotification);
	Lpeak.setLookAndFeel(lnf.get());
	addAndMakeVisible(Lpeak);

	LpeakVal.setText(juce::String(mindB), juce::dontSendNotification);
	LpeakVal.setLookAndFeel(lnf.get());
	addAndMakeVisible(LpeakVal);

	LfftSize.setText("Transform Size", juce::dontSendNotification);
	LfftSize.setLookAndFeel(lnf.get());
	addAndMakeVisible(LfftSize);

	LfftSizeVal.setText(juce::String(audioProcessor.N), juce::dontSendNotification);
	LfftSizeVal.setLookAndFeel(lnf.get());
	addAndMakeVisible(LfftSizeVal);

	LxScale.setText("Logarithmic", juce::dontSendNotification);
	LxScale.setLookAndFeel(lnf.get());
	addAndMakeVisible(LxScale);

	BxScale.setLookAndFeel(lnf.get());
	BxScale.onStateChange = [this] {isLog = BxScale.getToggleState(); };
	BxScale.setClickingTogglesState(true);
	addAndMakeVisible(BxScale);
}

puannhiAudioProcessorEditor::~puannhiAudioProcessorEditor()
{
	CwinFunc.setLookAndFeel(nullptr);
	LwinFunc.setLookAndFeel(nullptr);
	Sratio.setLookAndFeel(nullptr);
	Lratio.setLookAndFeel(nullptr);
	Lpeak.setLookAndFeel(nullptr);
	LpeakVal.setLookAndFeel(nullptr);
	LfftSize.setLookAndFeel(nullptr);
	LfftSizeVal.setLookAndFeel(nullptr);
	BxScale.setLookAndFeel(nullptr);
	LxScale.setLookAndFeel(nullptr);
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
	area.removeFromTop(70);

	area.reduce(40, 30);
	SpectrogramArea = area;

	auto row1 = 10;
	auto row2 = 40;
	auto row3 = 70;

	LwinFunc.setBounds(40, row1, 120, 25);
	CwinFunc.setBounds(160, row1, 250, 25);
	Lpeak.setBounds(420, row1, 100, 25);
	LpeakVal.setBounds(520, row1, 80, 25);

	Lratio.setBounds(40, row2, 120, 25);
	Sratio.setBounds(160, row2, 250, 25);
	LfftSize.setBounds(420, row2, 100, 25);
	LfftSizeVal.setBounds(520, row2, 80, 25);

	LxScale.setBounds(40, row3, 120, 25);
	BxScale.setBounds(155, row3, 25, 25);

	width_f = SpectrogramArea.getWidth();
	height_f = SpectrogramArea.getHeight();

	width_i = SpectrogramArea.getWidth();
	height_i = SpectrogramArea.getHeight();

	offset_x = SpectrogramArea.getX();
	offset_y = SpectrogramArea.getY();

	lineGridSize = width_f / (float)audioProcessor.lineScopeSize;
	barGridSize = width_f / (float)audioProcessor.barScopeSize;
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
	if (isLog)
	{
		skew = 0.3f;
	}
	else
	{
		skew = 1.0f;
	}

	for (int i = 0; i < audioProcessor.N; i++)
	{
		// to compensate the data outside nyquist
		auto amplitude = std::abs(audioProcessor.OutputArray[i]) * 2;
		//auto angle = std::arg(audioProcessor.frameProcessArray[i]);
		audioProcessor.currentOutputArray[i] = amplitude;
		audioProcessor.previousOutputArray[i] = (ratio / 100.0f) * audioProcessor.currentOutputArray[i] + (1.0f - (ratio / 100.0f)) * audioProcessor.previousOutputArray[i];
	}

	// convert data disribution from linear into logarithm
	// for line graph
	for (int i = 0; i < audioProcessor.lineScopeSize; i++)
	{
		auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)audioProcessor.lineScopeSize) * skew);
		auto fftDataIndex = juce::jlimit(0, audioProcessor.N / 2, (int)(skewedProportionX * (float)audioProcessor.N * 0.5f));

		auto Ve = juce::Decibels::gainToDecibels(audioProcessor.previousOutputArray[fftDataIndex]);
		auto V0 = juce::Decibels::gainToDecibels((float)audioProcessor.N);

		auto level_limited = juce::jlimit(mindB, maxdB, Ve-V0);
		
		auto level = juce::jmap(level_limited, mindB, maxdB, 0.0f, 1.0f);
		
		audioProcessor.lineScopeData[i] = level;

		if (level_limited > max)
		{
			max = level_limited;
		}
	}

	// convert data disribution from linear into logarithm
	// for bar graph
	for (int i = 0; i < audioProcessor.barScopeSize; i++)
	{
		auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)audioProcessor.barScopeSize) * skew);
		auto fftDataIndex = juce::jlimit(0, audioProcessor.N / 2, (int)(skewedProportionX * (float)audioProcessor.N * 0.5f));

		auto Ve = juce::Decibels::gainToDecibels(audioProcessor.previousOutputArray[fftDataIndex]);
		auto V0 = juce::Decibels::gainToDecibels((float)audioProcessor.N);

		auto level_limited = juce::jlimit(mindB, maxdB, Ve - V0);

		auto level = juce::jmap(level_limited, mindB, maxdB, 0.0f, 1.0f);

		audioProcessor.barScopeData[i] = level;
	}
}

void puannhiAudioProcessorEditor::drawFrame(juce::Graphics& g)
{
	g.setColour(juce::Colours::grey);
	g.fillRect(offset_x, offset_y, width_f, height_f);

	// line graph
	for (int i = 1; i < audioProcessor.lineScopeSize; i++)
	{
		g.setColour(juce::Colours::antiquewhite);
		g.drawLine({ 
			offset_x + (float)juce::jmap(i - 1, 0, audioProcessor.lineScopeSize - 1, 0, width_i),
			offset_y + juce::jmap(audioProcessor.lineScopeData[i - 1], 0.0f, 1.0f, height_f, 0.0f),
			offset_x + (float)juce::jmap(i,     0, audioProcessor.lineScopeSize - 1, 0, width_i),
			offset_y + juce::jmap(audioProcessor.lineScopeData[i],     0.0f, 1.0f, height_f, 0.0f)
			});
	}

	// bar graph
	for (int i = 0; i < audioProcessor.barScopeSize; i++)
	{
		g.setColour(juce::Colours::greenyellow);
		auto val = juce::jmap(audioProcessor.barScopeData[i], 0.0f, 1.0f, 0.0f, height_f);
		auto rect = juce::Rectangle<float>(offset_x + i * barGridSize, offset_y + (height_f - val), barGridSize, height_f - (height_f - val));
		rect.reduce(2, 0);
		g.fillRect(rect);
	}

	// display decibel
	LpeakVal.setText(juce::String(max), juce::dontSendNotification);
}

void puannhiAudioProcessorEditor::drawCoordiante(juce::Graphics & g)
{
	// should be driven by gui event?
	drawAmplitude(g);
	drawFrequency(g);
}

void puannhiAudioProcessorEditor::drawAmplitude(juce::Graphics& g)
{
	// amplitude tick
	g.setColour(juce::Colours::antiquewhite);
	for (int i = 0; i < 11; i++)
	{
		auto y_pos = offset_y + (i * height_f / 10);
		auto level = 0 - i * 10;
		g.drawHorizontalLine(y_pos, offset_x, offset_x + width_f);
		g.setFont(g.getCurrentFont().withHeight(10.0f));
		g.drawText(juce::String(level) + juce::String("dB"), offset_x - 40, int(y_pos) - 12, 35, 25, juce::Justification::right, false);
	}
}

void puannhiAudioProcessorEditor::drawFrequency(juce::Graphics& g)
{
	// frequency tick
	g.setColour(juce::Colours::antiquewhite);
	// draw 1st vertical tick
	g.drawVerticalLine(offset_x, offset_y, offset_y + height_f);
	// draw last vertical tick
	g.drawVerticalLine(offset_x + width_f, offset_y, offset_y + height_f);

	int k_iter = 1 + audioProcessor.getSampleRate() / 2000;
	if (isLog)
	{
		k_iter = 21;
	}

	// k-interval
	for (int i = 1; i < k_iter; i++)
	{
		auto frequency = i * 1000.0f;
		float x_pos = offset_x + inverse_x(frequency) * width_f;

		// draw line
		g.drawVerticalLine(x_pos, offset_y, offset_y + height_f);

		// draw text
		if (i < 11)
		{
			g.drawText(juce::String(i) + juce::String("k"), int(x_pos) - 16, offset_y + height_f, 30, 25, juce::Justification::centred, false);
		}
		else if (i == 12 || i == 15 || i == 20)
		{
			g.drawText(juce::String(i) + juce::String("k"), int(x_pos) - 16, offset_y + height_f, 30, 25, juce::Justification::centred, false);
		}
	}

	auto color = juce::Colours::antiquewhite.withAlpha(0.7f);
	g.setColour(color);
	// hundred-interval
	for (int i = 1; i < 10; i++)
	{
		auto frequency = i * 100.0f;
		float x_pos = offset_x + inverse_x(frequency) * width_f;

		// draw dashed line
		auto line = juce::Line<float>(x_pos, offset_y, x_pos, offset_y + height_f);
		float arr[] = { 3.0f, 6.0f };
		g.drawDashedLine(line, arr, 2);
		// draw line
		// g.drawVerticalLine(x_pos, offset_y, offset_y + height_f);
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

	auto lineGridSize = width / (float)audioProcessor.lineScopeSize;
	for (int i = 0; i < audioProcessor.lineScopeSize; i++)
	{
		auto val = juce::jmap(i / (float)audioProcessor.lineScopeSize, 0.0f, 1.0f, 0.0f, height_f);
		g.setColour(juce::Colours::greenyellow);
		g.fillRect(offset_x + i * lineGridSize, offset_y + (height_f - val), lineGridSize, height_f - (height_f - val));
	}
}

float puannhiAudioProcessorEditor::inverse_x(float frequency)
{
	auto fftDataIndex = frequency * audioProcessor.N / audioProcessor.getSampleRate();
	auto skewedProportionX = fftDataIndex * 2 / audioProcessor.N;
	return 1 - std::powf(1 - skewedProportionX, 1 / skew);
}
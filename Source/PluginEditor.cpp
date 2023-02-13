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

	//addAndMakeVisible(s_input_fftSize);
	//s_input_fftSize.setText(juce::String(audioProcessor.N));

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

	l_input_fftSize.setBounds(350, 30, 150, 30);
	//s_input_fftSize.setBounds(320, 30, 150, 30);
	
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
	float mindB = -100.0f;
	float maxdB = 6.0f;
	
	for (int i = 0; i < audioProcessor.N; i++)
	{
		auto amplitude = std::abs(audioProcessor.OutputArray[i]);
		//auto angle = std::arg(audioProcessor.frameProcessArray[i]);
		audioProcessor.currentOutputArray[i] = amplitude;
		audioProcessor.previousOutputArray[i] = Ratio * audioProcessor.currentOutputArray[i] + (1- Ratio)*audioProcessor.previousOutputArray[i];
	}

	//DrawFFTSpectrumDivide(audioProcessor.drawFFT, audioProcessor.target_frequency, audioProcessor.TargetFreqNum, audioProcessor.N, audioProcessor.input_sample_rate, audioProcessor.previousOutputArray, audioProcessor.DivideUnit);

	for (int i = 0; i < audioProcessor.scopeSize; ++i)
	{
		auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)audioProcessor.scopeSize) * 0.2f);
		auto fftDataIndex = juce::jlimit(0, audioProcessor.N / 2, (int)(skewedProportionX * (float)audioProcessor.N * 0.5f));
		auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(audioProcessor.previousOutputArray[fftDataIndex]) - juce::Decibels::gainToDecibels((float)audioProcessor.N)), mindB, maxdB, 0.0f, 1.0f);
		audioProcessor.scopeData[i] = level;
	}
}

void puannhiAudioProcessorEditor::drawFrame(juce::Graphics& g)
{
	auto width = SpectrogramArea.getWidth();
	auto height = SpectrogramArea.getHeight();

	float threthold1 = 0.28;
	float threthold2 = 0.47;

	float charwidth = 6;
	float xwidth = charwidth / 2 + 1;

	for (int i = 1; i < audioProcessor.scopeSize; ++i)
	{
		g.setColour(juce::Colours::antiquewhite);
		g.drawLine({ 
			(float)juce::jmap(i - 1, 0, audioProcessor.scopeSize - 1, 0, width),
			juce::jmap(audioProcessor.scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
			(float)juce::jmap(i,     0, audioProcessor.scopeSize - 1, 0, width),
			juce::jmap(audioProcessor.scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) 
			});

		//if (audioProcessor.scopeData[i] >= threthold1)
		//{
		//	if (audioProcessor.scopeData[i] >= threthold2)
		//	{
		//		g.setColour(juce::Colours::red);
		//		g.drawLine(SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(audioProcessor.scopeData[i], 0.0f, 1.0f, (float)height, 0.0f),
		//			SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap((audioProcessor.scopeData[i] - threthold2), 0.0f, 1.0f, (float)height, 0.0f), charwidth);

		//		g.setColour(juce::Colours::yellow);
		//		g.drawLine(SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(threthold1, 0.0f, 1.0f, (float)height, 0.0f),
		//			SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(threthold2, 0.0f, 1.0f, (float)height, 0.0f), charwidth);
		//		g.setColour(juce::Colours::greenyellow);
		//		g.drawLine(SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(0.0f, 0.0f, 1.0f, (float)height, 0.0f),
		//			SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(threthold1, 0.0f, 1.0f, (float)height, 0.0f), charwidth);

		//	}
		//	else
		//	{
		//		g.setColour(juce::Colours::yellow);
		//		g.drawLine(SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(audioProcessor.scopeData[i], 0.0f, 1.0f, (float)height, 0.0f),
		//			SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(audioProcessor.scopeData[i] - threthold1, 0.0f, 1.0f, (float)height, 0.0f), charwidth);
		//		g.setColour(juce::Colours::greenyellow);
		//		g.drawLine(SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(0.0f, 0.0f, 1.0f, (float)height, 0.0f),
		//			SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//			SpectrogramArea.getY() + juce::jmap(threthold1, 0.0f, 1.0f, (float)height, 0.0f), charwidth);
		//	}


		//}
		//else
		//{
		//	g.setColour(juce::Colours::greenyellow);
		//	g.drawLine(SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//		SpectrogramArea.getY() + juce::jmap(audioProcessor.scopeData[i], 0.0f, 1.0f, (float)height, 0.0f),
		//		SpectrogramArea.getX() + xwidth + (float)juce::jmap(i, 0, audioProcessor.scopeSize, 1, width),
		//		SpectrogramArea.getY() + juce::jmap(0.0f, 0.0f, 1.0f, (float)height, 0.0f), charwidth);
		//}

	}
}

void puannhiAudioProcessorEditor::drawCoordiante(juce::Graphics & g)
{
	auto area = SpectrogramArea;
	auto width = SpectrogramArea.getWidth();
	g.setFont(12.0f);
	g.setColour(juce::Colours::blue);
	auto deltaX=(float)juce::jmap(1 * 10, 0, audioProcessor.scopeSize, 0, width);
	
	for (int i = 0; i < 10; ++i)
	{
		auto x = area.getX() + (float)juce::jmap(i, 0, 10,0, width);
		g.setColour(juce::Colours::whitesmoke);
		g.drawFittedText(juce::String((audioProcessor.target_frequency[i] < 1000.0f) ? juce::String(audioProcessor.target_frequency[i]) + " Hz" : juce::String(audioProcessor.target_frequency[i] / 1000.0f) + " kHz"),
			juce::roundToInt(x + deltaX/4), area.getBottom() + 8, 40, 15, juce::Justification::left, 1);
		if (i >= 1)
		{
			g.setColour(juce::Colours::whitesmoke);
			g.drawVerticalLine(juce::roundToInt(area.getX() + i / 10.0f * area.getWidth()), area.getY(), area.getBottom());
		}
	}
	
	g.setColour(juce::Colours::whitesmoke);
	g.drawFittedText("  6 dB", area.getX() - 35, area.getY() + 2 - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("  0 dB", area.getX() - 35, juce::roundToInt(area.getY() + 0.056 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-10 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.15 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-30 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.34 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-40 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.43 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-50 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.53 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-60 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.62 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-70 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.72 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-80 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.81 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-90 dB", area.getX() - 40, juce::roundToInt(area.getY() + 0.90 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);
	g.drawFittedText("-100 dB", area.getX() - 40, juce::roundToInt(area.getY() + 1.0 * area.getHeight()) - 5, 50, 14, juce::Justification::left, 1);

	g.setColour(juce::Colours::whitesmoke);
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.00 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.056 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.15 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.34 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.43 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.53 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.62 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.72 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.81 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 0.90 * area.getHeight()), area.getX(), area.getRight());
	g.drawHorizontalLine(juce::roundToInt(area.getY() + 1.0 * area.getHeight()), area.getX(), area.getRight());
}


void puannhiAudioProcessorEditor::FindIndex(int numberLeft, int numberRight, int NumberFrequecy, int* Index)
{
	int StartIndex;
	int EndIndex;

	if (numberLeft % 2 == 0)
	{
		StartIndex = NumberFrequecy - numberLeft / 2;
	}
	else
	{
		StartIndex = NumberFrequecy - (numberLeft - 1) / 2;
	}
	if (numberRight % 2 == 0)
	{
		EndIndex = NumberFrequecy + numberRight / 2 - 1;
	}
	else
	{
		EndIndex = NumberFrequecy + (numberRight - 1) / 2;
	}
	Index[0] = StartIndex;
	Index[1] = EndIndex;
}

void puannhiAudioProcessorEditor::BandDivide(int FreqNumber, int TargetFreNum, float * OutputArray, int* FreqNumInterval, int* FreqNumArray, int N, int* Index)
{
	int StartIndex;
	int EndIndex;
	if (FreqNumber != 0 && FreqNumber != (TargetFreNum - 1))
	{
		FindIndex(FreqNumInterval[FreqNumber - 1], FreqNumInterval[FreqNumber], FreqNumArray[FreqNumber], Index);

	}
	else if (FreqNumber == 0)
	{
		if (FreqNumInterval[0] % 2 == 0)
		{
			Index[0] = 0;
			Index[1] = FreqNumArray[0] + FreqNumInterval[0] / 2 - 1;
		}
		else
		{
			Index[0] = 0;
			Index[1] = FreqNumArray[0] + (FreqNumInterval[0] - 1) / 2;
		}
	}
	else if (FreqNumber == (TargetFreNum - 1))
	{
		if (FreqNumInterval[TargetFreNum - 2] % 2 == 0)
		{
			Index[0] = FreqNumArray[TargetFreNum - 1] - FreqNumInterval[TargetFreNum - 2] / 2;
			Index[1] = N / 2;
		}
		else
		{
			Index[0] = FreqNumArray[TargetFreNum - 1] - (FreqNumInterval[TargetFreNum - 2] - 1) / 2;
			Index[1] = N / 2;
		}
	}
}

void puannhiAudioProcessorEditor::BandInternalDivide(int NumTargetFreNum, int DivideUnit, int StartIndex, int EndIndex, float* OutputArray, float* drawArray)
{
	int count = EndIndex - StartIndex + 1;
	int quotient = count / DivideUnit; //shang 39/10=3;
	int remainder = count % DivideUnit; //yushu  39/10=9;
	int startIndex = StartIndex;

	float* tempdrawdata = new float[DivideUnit];
	for (int i = 0; i < DivideUnit; ++i)
	{
		tempdrawdata[i] = 0;

		if (remainder != 0)
		{
			for (int j = startIndex; j < startIndex + quotient + 1; j++)
			{
				tempdrawdata[i] += OutputArray[j];
			}
			tempdrawdata[i] /= (quotient + 1);
			--remainder;
			startIndex += quotient + 1;
		}
		else
		{
			for (int j = startIndex; j < startIndex + quotient; j++)
			{
				tempdrawdata[i] += OutputArray[j];
			}
			startIndex += quotient;
			tempdrawdata[i] /= quotient;
		}
		drawArray[NumTargetFreNum*DivideUnit + i] = tempdrawdata[i];
	}
}

void puannhiAudioProcessorEditor::DrawFFTSpectrumDivide(float* drawArray, float* TargetFrequency, int TargetFreNum, int N, int fs, float* OutputArray, int DivideUnit)
{
	int* FreqNumArray = new int[TargetFreNum];
	int* FreqNumInterval = new int[TargetFreNum - 1];
	int  Index[2] = { 0 };


	for (int i = 0; i < TargetFreNum; i++)
	{
		FreqNumArray[i] = std::round(TargetFrequency[i] * N / fs);
	}

	for (int i = 0; i < TargetFreNum - 1; i++)
	{
		FreqNumInterval[i] = FreqNumArray[i + 1] - FreqNumArray[i];
	}

	int StartIndex, EndIndex;
	for (int i = 0; i < TargetFreNum; ++i)
	{

		BandDivide(i, TargetFreNum, OutputArray, FreqNumInterval, FreqNumArray, N, Index);
		StartIndex = Index[0];
		EndIndex = Index[1];
		BandInternalDivide(i, DivideUnit, StartIndex, EndIndex, OutputArray, drawArray);
	}
}
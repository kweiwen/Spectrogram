/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
puannhiAudioProcessor::puannhiAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

puannhiAudioProcessor::~puannhiAudioProcessor()
{
	delete[] InputArray;
	delete[] OutputArray;
	delete[] lineScopeData;
	delete[] previousOutputArray;
	delete[] currentOutputArray;
}

//==============================================================================
const juce::String puannhiAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool puannhiAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool puannhiAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool puannhiAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double puannhiAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int puannhiAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int puannhiAudioProcessor::getCurrentProgram()
{
    return 0;
}

void puannhiAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String puannhiAudioProcessor::getProgramName (int index)
{
    return {};
}

void puannhiAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void puannhiAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	forwardFFT = new juce::dsp::FFT(log2(N));
	input_sample_rate = sampleRate;

	circularbuffer.createCircularBuffer(N);
	circularbuffer.flushBuffer();

	//juce::zeromem(OutputArray, sizeof(std::complex<float>)*N);
	juce::zeromem(previousOutputArray, sizeof(float)*N);
	for (int i = 0; i < N; i++)
	{
		previousOutputArray[i] = juce::Decibels::gainToDecibels(previousOutputArray[i] / (N / 1));
	}

	for (int i = 0; i < lineScopeSize; i++)
	{
		lineScopeData[i] = 0.0f;
	}

	for (int i = 0; i < barScopeSize; i++)
	{
		barScopeData[i] = 0.0f;
	}
}

void puannhiAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool puannhiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
   // if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
   //  && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
   //     return false;

   // // This checks if the input layout matches the output layout
   //#if ! JucePlugin_IsSynth
   // if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
   //     return false;
   //#endif

    return true;
  #endif
}
#endif

void puannhiAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	auto channelDataL = buffer.getWritePointer(0);
	auto channelDataR = buffer.getWritePointer(1);

	for (auto i = 0; i < buffer.getNumSamples(); ++i)
	{
		//auto data = (channelDataL[i] + channelDataR[i]) * 0.5;
		auto data = (channelDataL[i]);
		circularbuffer.writeBuffer(data);
	}

	if (WindowTag == 1)
	{
		for (int i = 0; i < N; i++)
		{
			InputArray[i] = circularbuffer.readBuffer(N - i);
		}
	}
	else if(WindowTag == 2)
	{
		for (int i = 0; i < N; i++)
		{
			InputArray[i] = (circularbuffer.readBuffer(N - i))*0.5*(1 - cos(2 * M_PI*i / (N - 1)));

		}
	}
	else if (WindowTag == 3)
	{
		for (int i = 0; i < N; i++)
		{
			InputArray[i] = (circularbuffer.readBuffer(N - i))*(0.54-0.46*cos(2*M_PI*i/ (N - 1)));

		}
	}
	else if (WindowTag == 4)
	{
		for (int i = 0; i < N; i++)
		{
			InputArray[i] = (circularbuffer.readBuffer(N - i))*(0.42 - 0.5*cos(2 * M_PI*i / (N - 1)) + 0.08*cos(4 * M_PI*i / (N - 1)));

		}
	}
	else if (WindowTag == 5)
	{
		for (int i = 0; i < N; i++)
		{
			if (i <= N / 2)
			{
				InputArray[i] = (circularbuffer.readBuffer(N - i))*(2*i/ (N - 1));
			}
			else
			{
				InputArray[i] = (circularbuffer.readBuffer(N - i))*(2-2*i/(N-1));
			}
		}
	}

	if (!nextBlockReady)
	{
		forwardFFT->perform(InputArray, OutputArray, false);
		nextBlockReady = true;
	}
}

//==============================================================================
bool puannhiAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* puannhiAudioProcessor::createEditor()
{
    return new puannhiAudioProcessorEditor (*this);
}

//==============================================================================
void puannhiAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void puannhiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new puannhiAudioProcessor();
}

#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float kTwoPi = juce::MathConstants<float>::twoPi;

juce::AudioProcessorValueTreeState::ParameterLayout DevilFanAudioProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "mode", 1 }, "Mode",
                 StringArray { "Chorus", "Vibrato", "Tremolo", "Rotary", "Fan" }, 4));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "rate", 1 }, "Rate",
                 NormalisableRange<float> (0.1f, 20.0f, 0.01f, 0.4f), 5.0f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "depth", 1 }, "Depth",
                 NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.6f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "width", 1 }, "Width",
                 NormalisableRange<float> (0.0f, 2.0f, 0.001f), 1.0f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
                 NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));

    return { p.begin(), p.end() };
}

DevilFanAudioProcessor::DevilFanAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool DevilFanAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == l.getMainInputChannelSet();
}

void DevilFanAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    delayBuf.setSize (2, (int) (sr * 0.06) + 8);
    delayBuf.clear();
    writePos = 0;

    auto init = [this] (juce::SmoothedValue<float>& s, const char* id)
    {
        s.reset (sr, 0.03);
        s.setCurrentAndTargetValue (apvts.getRawParameterValue (id)->load());
    };
    init (sRate, "rate"); init (sDepth, "depth"); init (sWidth, "width"); init (sMix, "mix");
}

float DevilFanAudioProcessor::readDelay (int ch, float delaySamples) const
{
    const int size = delayBuf.getNumSamples();
    float rp = (float) writePos - delaySamples;
    while (rp < 0.0f) rp += (float) size;

    const int i0 = ((int) rp) % size;
    const int i1 = (i0 + 1) % size;
    const float frac = rp - std::floor (rp);
    const float* d = delayBuf.getReadPointer (ch);
    return d[i0] + frac * (d[i1] - d[i0]);
}

void DevilFanAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    const int mode = (int) apvts.getRawParameterValue ("mode")->load();

    sRate.setTargetValue  (apvts.getRawParameterValue ("rate")->load());
    sDepth.setTargetValue (apvts.getRawParameterValue ("depth")->load());
    sWidth.setTargetValue (apvts.getRawParameterValue ("width")->load());
    sMix.setTargetValue   (apvts.getRawParameterValue ("mix")->load());

    // Phase offset of the right channel against the left one, per mode
    static const float rightOffset[5] = { 0.25f, 0.25f, 0.25f, 0.5f, 0.125f };
    const float msToSamples = (float) sr * 0.001f;

    float* chan[2] = { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        const float rate  = sRate.getNextValue();
        const float depth = sDepth.getNextValue();
        const float width = sWidth.getNextValue();
        const float mix   = sMix.getNextValue();

        phase += rate / (float) sr;
        if (phase >= 1.0f) phase -= 1.0f;

        const float inL = chan[0][i];
        const float inR = chan[1] != nullptr ? chan[1][i] : inL;
        delayBuf.setSample (0, writePos, inL);
        delayBuf.setSample (1, writePos, inR);

        float out[2];
        for (int c = 0; c < 2; ++c)
        {
            float p = phase + (c == 0 ? 0.0f : rightOffset[mode]);
            if (p >= 1.0f) p -= 1.0f;
            const float s   = std::sin (kTwoPi * p);   // -1..1
            const float uni = 0.5f + 0.5f * s;         // 0..1
            const float in  = c == 0 ? inL : inR;
            float wet = in;

            switch (mode)
            {
                case 0: // Chorus: delayed copy blended with the dry signal
                    wet = 0.5f * (in + readDelay (c, (15.0f + depth * 8.0f * s) * msToSamples));
                    break;
                case 1: // Vibrato: delayed signal only, so the pitch wobbles
                    wet = readDelay (c, (3.0f + depth * 2.5f * s) * msToSamples);
                    break;
                case 2: // Tremolo: the volume goes up and down
                    wet = in * (1.0f - depth * uni);
                    break;
                case 3: // Rotary: Doppler wobble + opposite volume on each side
                    wet = readDelay (c, (2.0f + depth * 1.5f * s) * msToSamples)
                          * (1.0f - depth * 0.45f * (1.0f - uni));
                    break;
                default: // Fan: blades "cut" the sound in short dips + small wobble
                    wet = readDelay (c, (1.5f + depth * 0.8f * s) * msToSamples)
                          * (1.0f - depth * std::pow (1.0f - uni, 3.0f));
                    break;
            }
            out[c] = in * (1.0f - mix) + wet * mix;
        }

        // Width (mid/side): 0 = mono, 1 = normal, 2 = extra wide
        const float mid  = 0.5f * (out[0] + out[1]);
        const float side = 0.5f * (out[0] - out[1]) * width;
        chan[0][i] = mid + side;
        if (chan[1] != nullptr) chan[1][i] = mid - side;

        writePos = (writePos + 1) % delayBuf.getNumSamples();
    }

    lfoPhaseForUI.store (phase);
}

juce::AudioProcessorEditor* DevilFanAudioProcessor::createEditor()
{
    return new DevilFanAudioProcessorEditor (*this);
}

void DevilFanAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void DevilFanAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DevilFanAudioProcessor();
}

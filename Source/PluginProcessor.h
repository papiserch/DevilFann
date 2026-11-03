#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class DevilFanAudioProcessor : public juce::AudioProcessor
{
public:
    DevilFanAudioProcessor();
    ~DevilFanAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Devil Fan"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> lfoPhaseForUI { 0.0f };   // the devil reads this to move in time with the effect

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    float readDelay (int channel, float delaySamples) const;

    juce::AudioBuffer<float> delayBuf;
    int writePos = 0;
    double sr = 44100.0;
    float phase = 0.0f;
    juce::SmoothedValue<float> sRate, sDepth, sWidth, sMix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DevilFanAudioProcessor)
};

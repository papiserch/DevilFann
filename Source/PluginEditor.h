#pragma once
#include "PluginProcessor.h"

class DevilFanAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit DevilFanAudioProcessorEditor (DevilFanAudioProcessor&);
    ~DevilFanAudioProcessorEditor() override { stopTimer(); }

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawDevil (juce::Graphics&, juce::Rectangle<float> area);

    DevilFanAudioProcessor& proc;

    juce::ComboBox modeBox;
    juce::Slider rate, depth, width, mix;
    juce::Label rateL, depthL, widthL, mixL;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SA> rateA, depthA, widthA, mixA;
    std::unique_ptr<CA> modeA;

    float t = 0.0f, fanAngle = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DevilFanAudioProcessorEditor)
};

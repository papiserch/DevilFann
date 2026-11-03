#include "PluginEditor.h"

static constexpr float kTwoPi = juce::MathConstants<float>::twoPi;

DevilFanAudioProcessorEditor::DevilFanAudioProcessorEditor (DevilFanAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter ("mode")))
        modeBox.addItemList (choice->choices, 1);
    addAndMakeVisible (modeBox);

    auto setup = [this] (juce::Slider& s, juce::Label& l, const juce::String& name)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffff7a1a));
        s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff3a0a0a));
        s.setColour (juce::Slider::thumbColourId, juce::Colours::white);
        s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible (s);
        addAndMakeVisible (l);
    };
    setup (rate, rateL, "Rate");   rate.setTextValueSuffix (" Hz");
    setup (depth, depthL, "Depth");
    setup (width, widthL, "Width");
    setup (mix, mixL, "Mix");

    modeA  = std::make_unique<CA> (proc.apvts, "mode", modeBox);
    rateA  = std::make_unique<SA> (proc.apvts, "rate", rate);
    depthA = std::make_unique<SA> (proc.apvts, "depth", depth);
    widthA = std::make_unique<SA> (proc.apvts, "width", width);
    mixA   = std::make_unique<SA> (proc.apvts, "mix", mix);

    setSize (560, 460);
    startTimerHz (60);
}

void DevilFanAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();
    modeBox.setBounds (16, 16, 120, 28);

    auto knobs = b.removeFromBottom (150).reduced (10, 0);
    const int w = knobs.getWidth() / 4;
    juce::Label* labels[4]  = { &rateL, &depthL, &widthL, &mixL };
    juce::Slider* sliders[4] = { &rate, &depth, &width, &mix };
    for (int i = 0; i < 4; ++i)
    {
        auto col = knobs.removeFromLeft (w);
        labels[i]->setBounds (col.removeFromTop (22));
        sliders[i]->setBounds (col.reduced (6, 0));
    }
}

void DevilFanAudioProcessorEditor::timerCallback()
{
    t += 1.0f / 60.0f;
    const float r = proc.apvts.getRawParameterValue ("rate")->load();
    fanAngle += juce::jmin (0.08f + r * 0.03f, 0.9f);   // the fan blades spin faster with Rate
    repaint();
}

void DevilFanAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a0000), getWidth() * 0.5f, getHeight() * 0.6f,
                                             juce::Colour (0xff050000), 0.0f, 0.0f, true));
    g.fillAll();

    g.setColour (juce::Colour (0xffff7a1a));
    g.setFont (juce::Font (26.0f, juce::Font::bold | juce::Font::italic));
    g.drawText ("Devil Fan", getWidth() - 190, 14, 174, 34, juce::Justification::centredRight);

    drawDevil (g, getLocalBounds().withTrimmedBottom (150).toFloat());
}

static void drawFlame (juce::Graphics& g, float x, float baseY, float w, float h)
{
    juce::Path p;
    p.startNewSubPath (x - w * 0.5f, baseY);
    p.cubicTo (x - w * 0.9f, baseY - h * 0.4f, x - w * 0.1f, baseY - h * 0.6f, x, baseY - h);
    p.cubicTo (x + w * 0.1f, baseY - h * 0.6f, x + w * 0.9f, baseY - h * 0.3f, x + w * 0.5f, baseY);
    p.closeSubPath();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffff4a00), x, baseY,
                                             juce::Colour (0xffffd23a), x, baseY - h, false));
    g.fillPath (p);
}

void DevilFanAudioProcessorEditor::drawDevil (juce::Graphics& g, juce::Rectangle<float> area)
{
    using juce::Colour; using juce::Path; using juce::AffineTransform; using juce::PathStrokeType;

    const int mode = (int) proc.apvts.getRawParameterValue ("mode")->load();
    const float phase = proc.lfoPhaseForUI.load();
    const float sway = 0.07f * std::sin (kTwoPi * phase);
    const float bob = std::sin (t * 5.0f) * 5.0f;
    const float s = juce::jmin (area.getWidth() / 300.0f, area.getHeight() / 380.0f);

    // Drawing space is 300 x 380, then moved and scaled into the free area
    const auto base = AffineTransform::rotation (sway, 150.0f, 340.0f)
                          .translated (0.0f, bob)
                          .scaled (s)
                          .translated (area.getCentreX() - 150.0f * s, area.getY());

    const Colour skinA (0xffff5a3c), skinB (0xffb80d0d);
    const juce::ColourGradient skin (skinA, 110.0f, 110.0f, skinB, 230.0f, 260.0f, true);
    const auto stroke = [] (float w) { return PathStrokeType (w, PathStrokeType::curved, PathStrokeType::rounded); };

    g.saveState();
    g.addTransform (base);

    // shadow
    g.setColour (Colour (0x88000000));
    g.fillEllipse (88.0f, 352.0f, 124.0f, 18.0f);

    // tail wags with the effect
    const auto tailRot = AffineTransform::rotation (0.3f * std::sin (kTwoPi * phase), 190.0f, 278.0f);
    Path tail; tail.startNewSubPath (188.0f, 280.0f); tail.cubicTo (240.0f, 290.0f, 262.0f, 250.0f, 240.0f, 222.0f);
    tail.applyTransform (tailRot);
    Path tip; tip.addTriangle (232.0f, 214.0f, 252.0f, 218.0f, 242.0f, 236.0f);
    tip.applyTransform (tailRot);
    g.setGradientFill (skin);
    g.strokePath (tail, stroke (10.0f));
    g.fillPath (tip);

    // jeans and shoes
    Path jeans; jeans.startNewSubPath (112.0f, 270.0f);
    jeans.lineTo (190.0f, 270.0f); jeans.lineTo (196.0f, 335.0f); jeans.lineTo (156.0f, 335.0f);
    jeans.lineTo (150.0f, 305.0f); jeans.lineTo (144.0f, 335.0f); jeans.lineTo (106.0f, 335.0f);
    jeans.closeSubPath();
    g.setColour (Colour (0xff3b5f9e)); g.fillPath (jeans);
    g.setGradientFill (skin);
    g.fillEllipse (102.0f, 330.0f, 44.0f, 22.0f);
    g.fillEllipse (154.0f, 330.0f, 44.0f, 22.0f);

    // torso
    g.fillEllipse (111.0f, 206.0f, 80.0f, 84.0f);

    // arms: left one waves
    Path armL; armL.startNewSubPath (118.0f, 240.0f); armL.cubicTo (88.0f, 250.0f, 78.0f, 215.0f, 62.0f, 205.0f);
    armL.applyTransform (AffineTransform::rotation (0.3f * std::sin (t * 8.0f), 120.0f, 240.0f));
    Path armR; armR.startNewSubPath (184.0f, 240.0f); armR.cubicTo (214.0f, 250.0f, 226.0f, 232.0f, 232.0f, 214.0f);
    g.strokePath (armL, stroke (16.0f));
    g.strokePath (armR, stroke (16.0f));
    g.fillEllipse (223.0f, 197.0f, 22.0f, 22.0f);

    // flaming hair
    for (int i = 0; i < 3; ++i)
    {
        const float f = 1.0f + 0.18f * std::sin (t * 14.0f + (float) i * 2.1f);
        static const float xs[3] = { 122.0f, 150.0f, 188.0f };
        static const float ws[3] = { 34.0f, 46.0f, 36.0f };
        static const float hs[3] = { 70.0f, 98.0f, 66.0f };
        drawFlame (g, xs[i], 104.0f, ws[i], hs[i] * f);
    }

    // horns, ears, head
    g.setGradientFill (skin);
    Path horns; horns.addTriangle (92.0f, 120.0f, 84.0f, 80.0f, 122.0f, 104.0f);
    horns.addTriangle (208.0f, 120.0f, 216.0f, 80.0f, 178.0f, 104.0f);
    horns.addTriangle (76.0f, 168.0f, 38.0f, 140.0f, 84.0f, 138.0f);
    horns.addTriangle (224.0f, 168.0f, 262.0f, 140.0f, 216.0f, 138.0f);
    g.fillPath (horns);
    g.fillEllipse (70.0f, 93.0f, 160.0f, 144.0f);

    // sunglasses
    g.setColour (Colour (0xff18233f));
    g.fillRoundedRectangle (88.0f, 142.0f, 56.0f, 38.0f, 16.0f);
    g.fillRoundedRectangle (156.0f, 142.0f, 56.0f, 38.0f, 16.0f);
    g.fillRect (144.0f, 152.0f, 12.0f, 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.fillEllipse (99.0f, 149.0f, 18.0f, 8.0f);
    g.fillEllipse (167.0f, 149.0f, 18.0f, 8.0f);

    // nose
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillEllipse (143.0f, 183.0f, 14.0f, 10.0f);

    // mouth: in Fan mode it opens and closes like singing into a fan
    const bool fanMode = (mode == 4);
    const float open = fanMode ? 18.0f + 10.0f * std::abs (std::sin (t * 18.0f)) : 0.0f;
    Path mouth; mouth.startNewSubPath (104.0f, 200.0f);
    mouth.quadraticTo (150.0f, 250.0f + open, 198.0f, 196.0f);
    mouth.quadraticTo (150.0f, 210.0f, 104.0f, 200.0f);
    mouth.closeSubPath();
    g.setColour (fanMode ? Colour (0xff5a0000) : juce::Colours::white);
    g.fillPath (mouth);
    if (fanMode) { g.setColour (juce::Colours::white); g.fillRect (122.0f, 205.0f, 56.0f, 7.0f); }
    g.setColour (Colour (0xff3a0000));
    g.strokePath (mouth, PathStrokeType (4.0f));

    // the desk fan, only in Fan mode; the blades spin faster with the Rate knob
    if (fanMode)
    {
        const float cx = 150.0f, cy = 205.0f;
        for (int i = 0; i < 4; ++i)
        {
            Path blade; blade.addEllipse (-9.0f, -48.0f, 18.0f, 48.0f);
            blade.applyTransform (AffineTransform::rotation (fanAngle + (float) i * juce::MathConstants<float>::halfPi)
                                      .translated (cx, cy));
            g.setColour (Colour (0x99a8d8ff));
            g.fillPath (blade);
        }
        g.setColour (Colour (0xffcfd6e0));
        g.drawEllipse (cx - 54.0f, cy - 54.0f, 108.0f, 108.0f, 3.0f);
        g.fillEllipse (cx - 8.0f, cy - 8.0f, 16.0f, 16.0f);
    }

    g.restoreState();
}

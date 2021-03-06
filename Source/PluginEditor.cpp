/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colour(97u, 18u, 167u));
    g.fillEllipse(bounds);

    g.setColour(Colour(255u, 154u, 1u));
    g.drawEllipse(bounds, 1.f);

    Path p;

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);


        auto sliderAngRad = jmap(sliderPos, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }

}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = this->getValue();
        if (val > 999.f)
        {
            val /= 1000.f;
            addK = true;
        }
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse; //this should not happen. bad.
    }

    if (this->suffix != "")
    {
        str << " ";
            if (addK)
            {
                str << "k";
            }
        str << suffix;
    }

    return str;
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::green);
    //g.drawRect(sliderBounds);


    getLookAndFeel().drawRotarySlider(g, 
                                    sliderBounds.getX(), 
                                    sliderBounds.getY(), 
                                    sliderBounds.getWidth(), 
                                    sliderBounds.getHeight(), 
                                    jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), 
                                    startAng, 
                                    endAng, 
                                    *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.toFloat().getWidth() * 0.5f;

    g.setColour(Colour(0u, 1722u, 1u));
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(1.f >= pos);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);


    return r;
}


//==============================================================================


SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    peakFreqSlider(*audioProcessor.apvts.getParameter("PeakFreq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("PeakGain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("PeakQuality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCutFreq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCutSlope"), "dB/Oct"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCutFreq"), "Hz"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCutSlope"), "dB/Oct"),
    peakFreqSliderAttachment(audioProcessor.apvts, "PeakFreq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "PeakGain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "PeakQuality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCutFreq", lowCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCutSlope", lowCutSlopeSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCutFreq", highCutFreqSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCutSlope", highCutSlopeSlider),
    responseCurve(p)

{

    peakFreqSlider.labels.add({ 0.f, "20Hz" });
    peakFreqSlider.labels.add({ 1.f, "20kHz" });

    peakGainSlider.labels.add({ 0.f, "-24dB" });
    peakGainSlider.labels.add({ 1.f, "+24dB" });

    peakGainSlider.labels.add({ 0.f, "-24dB" });
    peakGainSlider.labels.add({ 1.f, "+24dB" });

    peakQualitySlider.labels.add({ 0.f, "0.1" });
    peakQualitySlider.labels.add({ 1.f, "10" });

    lowCutFreqSlider.labels.add({ 0.f, "20Hz" });
    lowCutFreqSlider.labels.add({ 1.f, "20kHz" });

    highCutFreqSlider.labels.add({ 0.f, "20Hz" });
    highCutFreqSlider.labels.add({ 1.f, "20kHz" });

    lowCutSlopeSlider.labels.add({ 0.f, "-12dB" });
    lowCutSlopeSlider.labels.add({ 1.f, "-48dB" });

    highCutSlopeSlider.labels.add({ 0.f, "-12dB" });
    highCutSlopeSlider.labels.add({ 1.f, "-48dB" });


    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* comp : getComps() ){
        addAndMakeVisible(comp);
    }

    addAndMakeVisible(&responseCurve);

    setSize (600, 500);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{

}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> bounds = getLocalBounds();
    float hRatio = 33.f / 100.f; // JUCE_LIVE_CONSTANT(33) / 100.f;
    juce::Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

    bounds.removeFromTop(5);

    responseCurve.setBounds(responseArea);

    juce::Rectangle<int> lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    juce::Rectangle<int> lowSlopeArea = lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5);
    juce::Rectangle<int> highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    juce::Rectangle<int> highSlopeArea = highCutArea.removeFromTop(highCutArea.getHeight() * 0.5);

    lowCutFreqSlider.setBounds(lowCutArea);
    lowCutSlopeSlider.setBounds(lowSlopeArea);
    highCutFreqSlider.setBounds(highCutArea);
    highCutSlopeSlider.setBounds(highSlopeArea);

    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);

}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps() {
    std::vector<juce::Component*> comps = { &peakFreqSlider, &peakGainSlider, &peakQualitySlider, &lowCutFreqSlider, &highCutFreqSlider, &lowCutSlopeSlider, &highCutSlopeSlider };

    return comps;
}
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

    auto center = bounds.getCentre();

    Path p;

    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());

    p.addRectangle(r);

    jassert(rotaryStartAngle < rotaryEndAngle);

    auto sliderAngRad = jmap(sliderPos, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

    g.fillPath(p);
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 22.5f);
    auto endAng = degreesToRadians(180.f - 22.5f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::green);
    g.drawRect(sliderBounds);


    getLookAndFeel().drawRotarySlider(g, 
                                    sliderBounds.getX(), 
                                    sliderBounds.getY(), 
                                    sliderBounds.getWidth(), 
                                    sliderBounds.getHeight(), 
                                    jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), 
                                    startAng, 
                                    endAng, 
                                    *this);
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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* comp : getComps() ){
        addAndMakeVisible(comp);
    }

    addAndMakeVisible(&responseCurve);

    setSize (600, 400);
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
    juce::Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

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
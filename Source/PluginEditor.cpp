/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"




//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
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
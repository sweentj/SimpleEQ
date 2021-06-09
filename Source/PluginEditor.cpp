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
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCutSlope", highCutSlopeSlider)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps() ){
        addAndMakeVisible(comp);
    }

    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }
    startTimer(60);

    setSize (600, 400);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; ++i) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>()) {
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<0>()) {
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<1>()) {
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<2>()) {
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<3>()) {
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<0>()) {
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<1>()) {
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<2>()) {
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<3>()) {
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input) {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));

}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

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

void SimpleEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged = true;
    //parametersChanged.set(true);
}

void SimpleEQAudioProcessorEditor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting) {
    //we don't care about this boi
}

void SimpleEQAudioProcessorEditor::timerCallback() {
    if (parametersChanged.compareAndSetBool(false, true)) {
        //update the monochain from apvts
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());

        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
        //signal a repaint to update response curve
        repaint();
    }
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps() {
    std::vector<juce::Component*> comps = { &peakFreqSlider, &peakGainSlider, &peakQualitySlider, &lowCutFreqSlider, &highCutFreqSlider, &lowCutSlopeSlider, &highCutSlopeSlider };

    return comps;
}
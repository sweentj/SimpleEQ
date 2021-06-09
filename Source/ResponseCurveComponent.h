/*
  ==============================================================================

    ResponseCurveComponent.h
    Created: 9 Jun 2021 6:38:59pm
    Author:  tjswe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ResponseCurveComponent : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
    ResponseCurveComponent(SimpleEQAudioProcessor&);
    ~ResponseCurveComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    void timerCallback() override;

private:

    juce::Atomic<bool> parametersChanged{ false };

    MonoChain monoChain;

    SimpleEQAudioProcessor& audioProcessor;
};
/*
  ==============================================================================

    CurveAdjusterComponent.h
    Created: 10 Apr 2023 9:35:02pm
    Author:  Mason Self

  ==============================================================================
*/

#pragma once
#include "CurveAdjusterEditor.h"
//#include "../PluginProcessor.h"

class CurveAdjusterComponent : public juce::Component, public juce::Slider::Listener
{
public:
    
    CurveAdjusterComponent(int paramIndex, const juce::String& paramName, bool shouldDisplaySlider, float componentWidth, float componentHeight, float adjusterWidth, float adjusterHeight, AudioPluginAudioProcessor& p, CurveAdjusterProcessor& cP, bool receivesModulation);
    
    //this constructor adds args for display name, min, and max output
    CurveAdjusterComponent(int paramIndex, const juce::String& paramName, bool shouldDisplaySlider, float componentWidth, float componentHeight, float adjusterWidth, float adjusterHeight, AudioPluginAudioProcessor& p, CurveAdjusterProcessor& cP, bool receivesModulation, const juce::String& _displayName, const juce::String& minOutputName, const juce::String& maxOutputName);
    
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider*) override;
    
    float GetWidth();
    float GetHeight();
    
    CurveAdjusterEditor curveAdjusterEditor;
    juce::Slider slider;
    
private:
    AudioPluginAudioProcessor& audioProcessor;
    
    const float width, height;
    juce::String displayName, minOutput, maxOutput;
    bool shouldDisplayNameAndOutputRange {false};
    
public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
};



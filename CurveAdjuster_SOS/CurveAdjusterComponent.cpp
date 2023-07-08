/*
  ==============================================================================

    CurveAdjusterComponent.cpp
    Created: 10 Apr 2023 9:35:02pm
    Author:  Mason Self

  ==============================================================================
*/

#include "CurveAdjusterComponent.h"

CurveAdjusterComponent::CurveAdjusterComponent(int paramIndex, const juce::String& paramName, bool shouldDisplaySlider, float componentWidth, float componentHeight, float adjusterWidth, float adjusterHeight, AudioPluginAudioProcessor& p, CurveAdjusterProcessor& cP, bool receivesModulation)
:curveAdjusterEditor(adjusterWidth, adjusterHeight, cP, true, true, receivesModulation), audioProcessor(p), width(componentWidth), height(componentHeight)
{
    sliderAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (audioProcessor.apvts, paramName, slider);

    auto param = std::make_unique<juce::AudioParameterFloat*>(static_cast<juce::AudioParameterFloat*>(audioProcessor.getParameters().getUnchecked(paramIndex)));

    slider.setRange((*param)->range.start, (*param)->range.end, 0.00);
    slider.setValue((*param)->get());

    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    if (shouldDisplaySlider)
    {
        addAndMakeVisible(slider);
    }
    slider.addListener(this);
    
    addAndMakeVisible(curveAdjusterEditor);
}

CurveAdjusterComponent::CurveAdjusterComponent(int paramIndex, const juce::String& paramName, bool shouldDisplaySlider, float componentWidth, float componentHeight, float adjusterWidth, float adjusterHeight, AudioPluginAudioProcessor& p, CurveAdjusterProcessor& cP, bool receivesModulation, const juce::String& _displayName, const juce::String& minOutputName, const juce::String& maxOutputName)
:CurveAdjusterComponent(paramIndex, paramName,shouldDisplaySlider,componentWidth,componentHeight,adjusterWidth,adjusterHeight, p, cP, receivesModulation)
{
    displayName = _displayName;
    minOutput = minOutputName;
    maxOutput = maxOutputName;
    shouldDisplayNameAndOutputRange = true;
}

void CurveAdjusterComponent::paint (juce::Graphics& g)
{
    if (shouldDisplayNameAndOutputRange)
    {
        g.setColour(juce::Colours::white);
        g.setOpacity(0.8f);
        g.setFont(20.0f);
        g.drawFittedText(displayName, juce::Rectangle<int>({0, static_cast<int>(curveAdjusterEditor.GetHeight())}, {static_cast<int>(curveAdjusterEditor.GetWidth()), getHeight()} ), juce::Justification::centred, 1);
        g.setFont(10.0f);
        g.drawFittedText(maxOutput, juce::Rectangle<int>({getWidth() - 25, 0}, {getWidth(), 12}), juce::Justification::centred, 1);
        g.drawFittedText(minOutput, juce::Rectangle<int>({getWidth() - 25, static_cast<int>(curveAdjusterEditor.GetHeight()) - 12}, {getWidth(), static_cast<int>(curveAdjusterEditor.GetHeight())}), juce::Justification::centred, 1);
    }
}
void CurveAdjusterComponent::resized()
{
    curveAdjusterEditor.setBounds(0, 0, curveAdjusterEditor.GetWidth(), curveAdjusterEditor.GetHeight());
}

void CurveAdjusterComponent::sliderValueChanged(juce::Slider*)
{
    auto normalizedSliderValue = slider.getValue() / slider.getMaximum();
    curveAdjusterEditor.SetParamValue(normalizedSliderValue);
}

float CurveAdjusterComponent::GetWidth()
{
    return width;
}

float CurveAdjusterComponent::GetHeight()
{
    return height;
}

/*
  ==============================================================================

    CurveAdjusterComponent.cpp
    Created: 10 Apr 2023 9:35:02pm
    Author:  Mason Self

  ==============================================================================
*/

#include "CurveAdjusterComponent.h"

CurveAdjusterComponent::CurveAdjusterComponent(int paramIndex,
                                               const juce::String& paramName,
                                               bool shouldDisplaySlider,
                                               float componentWidth,
                                               float componentHeight,
                                               float adjusterWidth,
                                               float adjusterHeight,
                                               IAudioProcessor& p,
                                               CurveAdjusterProcessor& cP,
                                               bool receivesModulation)
: curveAdjusterEditor(adjusterWidth, adjusterHeight, cP, true, true, receivesModulation),
  slider(p, paramName, paramIndex),
width(componentWidth),
height(componentHeight)
{
    if (shouldDisplaySlider)
    {
        addAndMakeVisible(slider);
    }
    slider.addListener(this);
    
    addAndMakeVisible(curveAdjusterEditor);
}

CurveAdjusterComponent::CurveAdjusterComponent(int paramIndex,
                                               const juce::String& paramName,
                                               bool shouldDisplaySlider,
                                               float componentWidth,
                                               float componentHeight,
                                               float adjusterWidth,
                                               float adjusterHeight,
                                               IAudioProcessor& p,
                                               CurveAdjusterProcessor& cP,
                                               bool receivesModulation,
                                               const juce::String& _displayName,
                                               const juce::String& minOutputName,
                                               const juce::String& maxOutputName)
: CurveAdjusterComponent(paramIndex, paramName,shouldDisplaySlider,componentWidth,componentHeight,adjusterWidth,adjusterHeight, p, cP, receivesModulation)
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
    curveAdjusterEditor.setBounds(0, 0, static_cast<int>(curveAdjusterEditor.GetWidth()), static_cast<int>(curveAdjusterEditor.GetHeight()));
    slider.setBounds(5, curveAdjusterEditor.getBottom(), static_cast<int>(curveAdjusterEditor.GetWidth()) + 10, static_cast<int>(GetHeight() - curveAdjusterEditor.GetHeight() / 2.0f));
}

void CurveAdjusterComponent::sliderValueChanged(juce::Slider*)
{
    auto normalizedSliderValue = slider.getValue() / slider.getMaximum();
    curveAdjusterEditor.SetParamValue(normalizedSliderValue);
}

[[maybe_unused]] float CurveAdjusterComponent::GetWidth()
{
    return width;
}

float CurveAdjusterComponent::GetHeight()
{
    return height;
}

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
                                               bool minIsAdjustable,
                                               bool maxIsAdjustable,
                                               float componentWidth,
                                               float componentHeight,
                                               float adjusterWidth,
                                               float adjusterHeight,
                                               IAudioProcessor& p,
                                               CurveAdjusterProcessor& cP,
                                               bool receivesModulation)
: curveAdjusterEditor(adjusterWidth, adjusterHeight, cP, minIsAdjustable, maxIsAdjustable, receivesModulation),
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
                                               bool minIsAdjustable,
                                               bool maxIsAdjustable,
                                               float componentWidth,
                                               float componentHeight,
                                               float adjusterWidth,
                                               float adjusterHeight,
                                               IAudioProcessor& p,
                                               CurveAdjusterProcessor& cP,
                                               bool receivesModulation,
                                               const juce::String& _displayName,
                                               const juce::String& minOutputName,
                                               const juce::String& maxOutputName,
                                               juce::Colour _textColor,
                                               float _textOpacity)
: CurveAdjusterComponent(paramIndex, paramName,shouldDisplaySlider, minIsAdjustable, maxIsAdjustable, componentWidth,componentHeight,adjusterWidth,adjusterHeight, p, cP, receivesModulation)
{
    displayName = _displayName;
    minOutput = minOutputName;
    maxOutput = maxOutputName;
    textColor = _textColor;
    textOpacity = _textOpacity;
    shouldDisplayNameAndOutputRange = true;
}

void CurveAdjusterComponent::paint (juce::Graphics& g)
{
    if (shouldDisplayNameAndOutputRange)
    {
        g.setColour(textColor);
        g.setOpacity(textOpacity);
        g.setFont(20.0f);
        g.drawFittedText(displayName, juce::Rectangle<int>({0, getHeight() - 20}, {static_cast<int>(curveAdjusterEditor.GetWidth()), getHeight()} ), juce::Justification::centred, 1);
        g.setFont(10.0f);
        g.drawFittedText(maxOutput,
                         juce::Rectangle<int>(
                            {curveAdjusterEditor.getWidth(), 0},
                            {curveAdjusterEditor.getWidth() + 25, 12}),
                         juce::Justification::centred, 1);
        g.drawFittedText(minOutput,
                         juce::Rectangle<int>(
                                {curveAdjusterEditor.getWidth(), static_cast<int>(curveAdjusterEditor.GetHeight()) - 12},
                                {curveAdjusterEditor.getWidth() + 25, static_cast<int>(curveAdjusterEditor.GetHeight())}),
                        juce::Justification::centred, 1);
    }
}
void CurveAdjusterComponent::resized()
{
    curveAdjusterEditor.setBounds(0, 0, static_cast<int>(curveAdjusterEditor.GetWidth()), static_cast<int>(curveAdjusterEditor.GetHeight()));
    auto sliderHeight = 16;
    slider.setBounds(0, curveAdjusterEditor.getBottom(), curveAdjusterEditor.getWidth(), sliderHeight);
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

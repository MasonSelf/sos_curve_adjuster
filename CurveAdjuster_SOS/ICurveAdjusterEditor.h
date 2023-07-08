/*
  ==============================================================================

    ICurveAdjusterEditor.h
    Created: 19 Jan 2023 9:31:38pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class ICurveAdjusterEditor : public juce::Component,
                             public juce::Value::Listener
{
public:
     

    virtual void resized() override = 0;
    virtual void paint(juce::Graphics& g) override = 0;
    virtual void InitHandles() = 0;

    virtual float GetWidth() = 0;
    virtual float GetHeight() = 0;
private:

};
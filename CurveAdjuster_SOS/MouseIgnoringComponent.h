/*
  ==============================================================================

    MouseIgnoringComponent.h
    Created: 20 Jan 2023 5:35:36pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct MouseIgnoringComponent : public juce::Component
{
    MouseIgnoringComponent()
    {
        setInterceptsMouseClicks(false, false);
    }
};

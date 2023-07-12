/*
  ==============================================================================

    SmoothedValueManager.h
    Created: 24 Apr 2023 12:13:05pm
    Author:  Mason Self

  ==============================================================================
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class SmoothedValueManager
{
public:
    SmoothedValueManager(float initVal, double _rampLength)
    : value(initVal), ramplength(_rampLength)
    {
        smoothedVal.setCurrentAndTargetValue(initVal);
    }
    
    void Reset(double sampleRate)
    {
        smoothedVal.reset(sampleRate, ramplength);
    }
    float GetNextValue(float possibleNewTarget)
    {
        if (juce::approximatelyEqual(possibleNewTarget, smoothedVal.getTargetValue()))
        {
            smoothedVal.setTargetValue(possibleNewTarget);
        }
        value = smoothedVal.getNextValue();
        return value;
    }
    float value;
    const double ramplength;
private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedVal;
};

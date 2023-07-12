/*
  ==============================================================================

    ICurveAdjusterProcessor.h
    Created: 19 Jan 2023 9:31:50pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include "CurveAdjusterPointTypes.h"
#include "CurveAdjusterProcessorData.h"
#include <juce_audio_processors/juce_audio_processors.h>


namespace CurveAdjuster
{
    class ICurveAdjusterProcessor
    {
    public:
        virtual ~ICurveAdjusterProcessor() = default;
        

        virtual size_t GetNumConnectors() = 0;

        virtual void SaveState(juce::AudioProcessorValueTreeState& stateToAppendTo) = 0;
        virtual void LoadAndRemoveStateFromAPTVS(juce::ValueTree& apvtsTree) = 0;
        virtual const juce::Identifier& GetName() const = 0;

    protected:
        virtual void SetState(juce::ValueTree& curveAdjusterTree) = 0;
        virtual void RemoveThisCurveAdjusterTreeFromAPVTS(juce::ValueTree& apvtsTree, juce::ValueTree& curveAdjusterTree) = 0;


    };

}

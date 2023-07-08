/*
  ==============================================================================

    CurveAdjusterProcessor.h
    Created: 19 Jan 2023 9:30:11pm
    Author:  thegr

  ==============================================================================
*/

#pragma once

#include "ICurveAdjusterProcessor.h"
#include "SmoothedValueManager.h"
#include "DebugHelperFunctions.h"



namespace CurveAdjuster
{

    class CurveAdjusterProcessor : public ICurveAdjusterProcessor
    {
    public:

        CurveAdjusterProcessor(std::string n, float initVal, double smoothingIncrement, std::vector<ConnectorPoints> _connnectorPoints);
        CurveAdjusterProcessor(std::string n, float initVal, double smoothingIncrement); //default linear ramp up
        ~CurveAdjusterProcessor();

        int GetNumConnectors() override;

        void SaveState(juce::AudioProcessorValueTreeState& stateToAppendTo) override;
        void LoadAndRemoveStateFromAPTVS(juce::ValueTree& apvtsTree) override;
        const juce::Identifier& GetName() const override;


        float GetTranslatedOutput(float x);

        juce::Atomic<float> inputX {0.0f};
        
        std::atomic<bool> ReadyForGuiInit{ false } ;
        
        CurveAdjusterProcessorData data;
        
        SmoothedValueManager smoothedVal;

    protected:
        void SetState(juce::ValueTree& curveAdjusterTree) override;
        void RemoveThisCurveAdjusterTreeFromAPVTS(juce::ValueTree& treeapvtsTree, juce::ValueTree& curveAdjusterTree) override;
        float GetY_AtX(const AtomicConnector& c, float in_X);
        

    private:
        
        bool defaultDataAdded{ false }; 

        const juce::Identifier name;
        const juce::Identifier connectors_ID {"control_coordinates"};
        const juce::Identifier value_string_as_ID {"value"};
        
        //juce::Graphics g; //for path
        
    };

}

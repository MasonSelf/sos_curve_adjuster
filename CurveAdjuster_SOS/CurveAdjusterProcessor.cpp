/*
  ==============================================================================

    CurveAdjusterProcessor.cpp
    Created: 19 Jan 2023 9:30:11pm
    Author:  thegr

  ==============================================================================
*/

#include "CurveAdjusterProcessor.h"

namespace CurveAdjuster
{

CurveAdjusterProcessor::CurveAdjusterProcessor(std::string n, float initVal, double smoothingIncrement, std::vector<ConnectorPoints> _connnectorPoints)
:
smoothedVal(initVal, smoothingIncrement),
name(n)
{
    int connectorIndex = 0;
    for (auto& c : _connnectorPoints)
    {
        data[connectorIndex].startX = c.start.x;
        data[connectorIndex].startY = c.start.y;
        data[connectorIndex].controlX = c.control.x;
        data[connectorIndex].controlY = c.control.y;
        data[connectorIndex].endX = c.end.x;
        data[connectorIndex].endY = c.end.y;
        ++connectorIndex;
    }
    //this flag set after default values have been added
    defaultDataAdded = true;
}

//default linear ramp up
CurveAdjusterProcessor::CurveAdjusterProcessor(std::string n, float initVal, double smoothingIncrement)
: CurveAdjusterProcessor(n, initVal, smoothingIncrement, {{{0.0f, 0.0f}, {0.25f, 0.25f}, {1.0f, 1.0f}}})
{
}

CurveAdjusterProcessor::~CurveAdjusterProcessor()
{
}

size_t CurveAdjusterProcessor::GetNumConnectors()
{
    for (size_t i = 0; i < data.maxConnectors.load(); ++i)
    {
        if (data[i].endX == 1.0f)
        {
            return ++i;
        }
    }
    jassert(false); //there has to be a connector at the end!
    return 0; //TODO: make return type optional?
}


void CurveAdjusterProcessor::SaveState(juce::AudioProcessorValueTreeState& stateToAppendTo)
{
    //use this to clear the points if needed when debugging
    //return;
    
    //DBG("saving");
    juce::ValueTree temp
    { name, {},
        {
            { connectors_ID , {},}
        }
    };
    
    //iterate through any handles and add to the temp tree
    for (size_t i = 0; i < GetNumConnectors(); ++i)
    {
        //create tree for the point
        juce::Identifier connectorName = juce::String("connector" + std::to_string(i));
        juce::ValueTree connectorTree
        { connectorName, {},
            {
                {"startX", {{value_string_as_ID, data[i].startX.load()}}},
                {"startY", {{value_string_as_ID, data[i].startY.load()}}},
                {"controlX", {{value_string_as_ID, data[i].controlX.load()}}},
                {"controlY", {{value_string_as_ID, data[i].controlY.load()}}},
                {"endX", {{value_string_as_ID, data[i].endX.load()}}},
                {"endY", {{value_string_as_ID, data[i].endY.load()}}}
            }
        };
        
        //add pointTree to "handle coordinates" in temptree
        temp.getChildWithName(connectors_ID).appendChild(connectorTree, nullptr);
    }
    
    //DBG(temp.toXmlString());
    
    if (stateToAppendTo.state.getChildWithName(name).isValid())
    {
        stateToAppendTo.state.getChildWithName(name).copyPropertiesAndChildrenFrom(temp, nullptr);
    }
    else
    {
        stateToAppendTo.state.appendChild(temp, nullptr);
    }
}

void CurveAdjusterProcessor::LoadAndRemoveStateFromAPTVS(juce::ValueTree& apvtsTree)
{
    //dbug usage
    //return;
    
    //DBG("loading");
    auto curveAdjusterTree = apvtsTree.getChildWithName(name);
    if (curveAdjusterTree.isValid())
    {
        SetState(curveAdjusterTree);
        RemoveThisCurveAdjusterTreeFromAPVTS(apvtsTree, curveAdjusterTree);
    }
}

const juce::Identifier& CurveAdjusterProcessor::GetName() const
{
    return name;
}

float CurveAdjusterProcessor::GetTranslatedOutput(float in_X)
{
    //jassert(in_X >= 0.0f); //negative input is bad input!
    if (in_X <= 0.0f) in_X = 0.0f;
    
    //find points at which in_X lies between
    auto numConnectors = GetNumConnectors();
    for (size_t i = 0; i < numConnectors; ++i)
    {
        if (data[i].startX < in_X && data[i].endX > in_X)
        {
            return GetY_AtX(data[i], in_X);
        }
        
        //simplified path if in_X equals start or end of segment
        if (juce::approximatelyEqual(data[i].startX.load(), in_X))
        {
            return data[i].startY;
        }
        if (juce::approximatelyEqual(data[i].endX.load(), in_X))
        {
            return data[i].endY;
        }
    }

    //jassert(false); //should have found a compatible x!
    return 0.0f;
}

void CurveAdjusterProcessor::SetState(juce::ValueTree& curveAdjusterTree)
{
    auto setOfConnectorsChild = curveAdjusterTree.getChildWithName(connectors_ID);
    if (!setOfConnectorsChild.isValid())
    {
        return;
    }

    for (auto i = 0; i < setOfConnectorsChild.getNumChildren(); ++i)
    {
        auto connectorChild = setOfConnectorsChild.getChild(i);
        data[i].startX.store((float)connectorChild.getChildWithName("startX").getProperty(value_string_as_ID, -2.0));
        data[i].startY.store((float)connectorChild.getChildWithName("startY").getProperty(value_string_as_ID, -1.0));
        data[i].controlX.store((float)connectorChild.getChildWithName("controlX").getProperty(value_string_as_ID, -1.0));
        data[i].controlY.store((float)connectorChild.getChildWithName("controlY").getProperty(value_string_as_ID, -1.0));
        data[i].endX.store((float)connectorChild.getChildWithName("endX").getProperty(value_string_as_ID, -1.0));
        data[i].endY.store((float)connectorChild.getChildWithName("endY").getProperty(value_string_as_ID, -1.0));
    }
    
    ReadyForGuiInit = true;
}

void CurveAdjusterProcessor::RemoveThisCurveAdjusterTreeFromAPVTS(juce::ValueTree& apvtsTree, juce::ValueTree& curveAdjusterTree)
{
    apvtsTree.removeChild(curveAdjusterTree, nullptr);
}

float CurveAdjusterProcessor::GetY_AtX(const AtomicConnector& c, float in_X)
{
//    const pointType start {c.startX.load(), c.startY.load()};
//    const pointType control {c.controlX.load(), c.controlY.load()};
//    const pointType end {c.endX.load(), c.endY.load()};
    
    //jassert(start.x - 2.0f * control.x + end.x != 0.0f); //this should already be addressed
    

    
    juce::Path path;
    path.startNewSubPath(c.startX.load(), c.startY.load());
    path.quadraticTo(c.controlX.load(), c.controlY.load(), c.endX.load(), c.endY.load());

    juce::PathFlatteningIterator flatPath {path};
    juce::Line<float> verticalLineAtX {in_X, 0.0f, in_X, 1.0f};
    while (flatPath.next())
    {
        if (in_X >= flatPath.x1 && in_X <= flatPath.x2)
        {
            juce::Line<float> pathLine {flatPath.x1, flatPath.y1, flatPath.x2, flatPath.y2};
            return pathLine.getIntersection(verticalLineAtX).y;
        }
    }
    


    jassertfalse; //should have found something!
    return 0.0f;
//    float t = (start.x - control.x + SquareRootApproximation::ApproximateSquareRoot(in_X * start.x + in_X * end.x - 2.0f * in_X * control.x + juce::square(control.x) - start.x * end.x)) / (start.x - 2 * control.x + end.x);
//    pointType targetLineStart
//    {(1.0f - t) * start.x + t * control.x,
//        (1.0f - t) * start.y + t * control.y};
//    pointType targetLineEnd
//    {(1.0f - t) * control.x + t * end.x,
//        (1.0f - t) * control.y + t * end.y};
//    pointType target
//    {(1.0f - t) * targetLineStart.x + t * targetLineEnd.x,
//        (1.0f - t) * targetLineStart.y + t * targetLineEnd.y};
//
//    return target.y;
}

}

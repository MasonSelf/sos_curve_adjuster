/*
  ==============================================================================

    DebugHelperFunctions.h
    Created: 27 Jan 2023 9:21:05pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include <juce_graphics/juce_graphics.h>
namespace DebugHelp
{
    static void PrintVectorOfPoints(const std::vector<juce::Point<float>>& c)
    {
        std::string output;
        for (auto p : c)
        {
            output += " (" + std::to_string(p.x) + "," + std::to_string(p.y) + ")";
        }
        DBG(output);
    }

    static void PrintPoint(juce::Point<float>& p)
    {
        std::string output = std::to_string(p.x) + ", " + std::to_string(p.y);
        DBG(output);
    }

    static juce::String GetPointAsString(juce::Point<float>& p)
    {
        return std::to_string(p.x) + ", " + std::to_string(p.y);
    }

    static juce::String PrintBool(bool b)
    {
        return b ? "true" : "false";
    }
}

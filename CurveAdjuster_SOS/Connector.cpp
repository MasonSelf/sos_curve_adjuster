/*
  ==============================================================================

    Connector.cpp
    Created: 16 Apr 2023 9:56:52pm
    Author:  Mason Self

  ==============================================================================
*/
#pragma once
#include "Connector.h"

Connector::Connector(pointType _start, pointType _end)
: start(_start), end(_end)
{
    /*this constructor doesn't have a control point so we are
    setting a default control point for a straight line
     */
    control = {static_cast<float>(start.x + end.x) / 2.0f, static_cast<float>(start.y + end.y) / 2.0f};
    SetPath();
}

Connector::Connector(pointType _start, pointType _control, pointType _end)
: start(_start), control(_control), end(_end)
{
    SetPath();
}

Connector::Connector(const Connector& other)
{
    start = other.start;
    control = other.control;
    end = other.end;
    path = other.path;
}

void Connector::paint(juce::Graphics& g)
{
    g.setColour(mouseOver ? juce::Colours::red : juce::Colours::white);
    g.strokePath(path, juce::PathStrokeType(1.0f));
}

void Connector::AdjustStartPoint(pointType newStart)
{
    auto xChange = newStart.x - start.x;
    auto yChange = newStart.y - start.y;
    start = newStart;
    control.x += (xChange / 2.0f);
    control.y += (yChange / 2.0f);
    LimitControlPoint(control);
    SetPath();
    repaint();
}
void Connector::AdjustControlPoint(pointType newControlPoint)
{
    LimitControlPoint(newControlPoint);
    control = newControlPoint;
    SetPath();
    repaint();
}
void Connector::AdjustEndPoint(pointType newEnd)
{
    auto xChange = newEnd.x - end.x;
    auto yChange = newEnd.y - end.y;
    end = newEnd;
    control.x += (xChange / 2.0f);
    control.y += (yChange / 2.0f);
    LimitControlPoint(control);
    SetPath();
    repaint();
}

void Connector::SetPath()
{
    path.clear();
    path.startNewSubPath(start.x, start.y);
    path.quadraticTo(control.x, control.y, end.x, end.y);
}

void Connector::ForceMouseOverFalse()
{
    mouseOver = false;
    repaint();
}

Connector::ConnectorDirection Connector::GetConnectorDirection()
{
    return start.y > end.y ? up : down;
}

//limit range so path doesn't overlap itself
void Connector::LimitControlPoint(pointType& c)
{
    //x
    if (c.x < start.x)
    {
        c.x = start.x;
    }
    else if (c.x > end.x)
    {
        c.x = end.x;
    }
    
    //y
    if (GetConnectorDirection() == up)
    {
        c.y = LimitControlY_ForUpwardConnector(c.y);
    }
    else
    {
        c.y = LimitControlY_ForDownwardConnector(c.y);
    }
}


float Connector::LimitControlY_ForDownwardConnector(float inY)
{
    if (inY < start.y)
    {
        inY = start.y;
    }
    if (inY > end.y)
    {
        inY = end.y;
    }
    return inY;
}

float Connector::LimitControlY_ForUpwardConnector(float inY)
{
    if (inY > start.y)
    {
        inY = start.y;
    }
    if (inY < end.y)
    {
        inY = end.y;
    }
    return inY;
}

float Connector::GetY_AlongPath(float in_X)
{
    juce::PathFlatteningIterator flatPath {path};

    while (flatPath.next())
    {
        if (in_X >= flatPath.x1 && in_X <= flatPath.x2)
        {
            juce::Line<float> pathLine {flatPath.x1, flatPath.y1, flatPath.x2, flatPath.y2};
            juce::Line<float> verticalLineAtX {in_X, 0.0f, in_X, 1.0f};
            return pathLine.getIntersection(verticalLineAtX).y;
        }
    }
    jassertfalse; //should have found something!
    
    //safety
    return 0.0f;
}

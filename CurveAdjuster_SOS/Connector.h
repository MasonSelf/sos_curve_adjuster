/*
  ==============================================================================

    Connector.h
    Created: 16 Apr 2023 9:56:52pm
    Author:  Mason Self

  ==============================================================================
*/

#pragma once
#include "MouseIgnoringComponent.h"
#include "CurveAdjusterPointTypes.h"

using namespace CurveAdjuster;

class Connector : public MouseIgnoringComponent
{
public:
    using connectorsCollection = std::list<Connector>;
    using connectorsCollectionIterator = std::list<Connector>::iterator;
    
    Connector(pointType _start, pointType _end);
    Connector(pointType _start, pointType _control, pointType _end);
    Connector(const Connector&);
    
    void paint(juce::Graphics& g) override;
    
    void AdjustStartPoint(pointType newStart);
    void AdjustControlPoint(pointType newControlPoint);
    void AdjustEndPoint(pointType newEnd);
    void ForceMouseOverFalse();
    void SetPath();
    float GetY_AlongPath(float in_X); 
    
    pointType start, control, end;
    juce::Path path;
    bool mouseOver {false};
    
private:
    enum ConnectorDirection
    {
        up, down
    };
    
    ConnectorDirection GetConnectorDirection();
    float GetY_At_X_StraightLine(const pointType& _start, const pointType& _end, float _x);
    void LimitControlPoint(pointType& c);
    float LimitControlY_ForDownwardConnector(float inY);
    float LimitControlY_ForUpwardConnector(float inY);
};

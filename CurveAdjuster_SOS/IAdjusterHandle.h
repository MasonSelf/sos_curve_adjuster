/*
  ==============================================================================

    IHandle.h
    Created: 20 Jan 2023 5:12:38pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include "MouseIgnoringComponent.h"
#include "CurveAdjusterPointTypes.h"

namespace CurveAdjuster
{

    /*
    A Handle represents a connection point of segment of the path of a Curve Adjuster. 
    Depending on the implementation, it may be adjustable and serve as one means to adjust the path. 
    */
    class IAdjustmentHandle : public MouseIgnoringComponent
    {
    public:
        
        virtual void paint(juce::Graphics& g) override = 0;
        virtual void HandlePossibleMouseOver(const pointType mousePos) = 0;
        virtual bool GetIsMouseWithin() = 0;
        virtual pointType GetPos() = 0;
        virtual void SetPos(pointType) = 0;
        virtual float GetSize() = 0;
        virtual bool GetCanMoveHorizontally() = 0;
        virtual bool GetCanMoveVertically() = 0;
        virtual void ForceMouseWithinFalse() = 0;
        virtual void ForceMouseWithinTrue() = 0;
        virtual void SetSelectedForMultiSelect(bool) = 0;
        virtual bool GetIsSelectedInMultiSelect() = 0;

    private:
        virtual void DetermineMouseEnter(const pointType mousePos) = 0;
        virtual pointType OffsetPointForOrigin(pointType p) = 0;
        virtual pointType GetCenterFromOrigin() = 0;

    };

    using handleCollection = std::list<std::unique_ptr<IAdjustmentHandle>>;
    using handleCollectionIterator = std::list<std::unique_ptr<IAdjustmentHandle>>::iterator;
}

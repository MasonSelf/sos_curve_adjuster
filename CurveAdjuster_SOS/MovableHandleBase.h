/*
  ==============================================================================

    CurveAdjusterHandle.h
    Created: 20 Jan 2023 5:15:33pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include "IAdjusterHandle.h"
#include "CurveAdjusterPointTypes.h"

namespace CurveAdjuster
{

    class MovableHandleBase : public IAdjustmentHandle
    {
    public:
        MovableHandleBase(float diameter);
        MovableHandleBase(float diameter, pointType p);

        void paint(juce::Graphics& g) override;
        void HandlePossibleMouseOver(const pointType mousePos) override;

        bool GetIsMouseWithin() override;
        pointType GetPos() override;
        void SetPos(pointType) override;
        float GetSize() override;

        //these should be implemented in derived classes
        bool GetCanMoveHorizontally() override;
        bool GetCanMoveVertically() override;


        void ForceMouseWithinFalse() override;
        void ForceMouseWithinTrue() override;
        void SetSelectedForMultiSelect(bool shouldBeSelected) override;
        bool GetIsSelectedInMultiSelect() override;

    private:
        void DetermineMouseEnter(const pointType mousePos) override;
        pointType OffsetPointForOrigin(pointType p) override;
        pointType GetCenterFromOrgin() override;

        bool isMultiSelected{false};
        bool mouseIsWithin{ false };
        const float size{ 10.0f };
        const float halfSize{ 5.0f };
        const float mouseCushion{ 5.0f };
        pointType pos{ 0.0f, 0.0f };
    };
}

/*
  ==============================================================================

    StationaryHandle.h
    Created: 20 Jan 2023 5:54:06pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include "IAdjusterHandle.h"
namespace CurveAdjuster
{
    /* This handle is not reposition-able! Primarily intended for start
    or end values in a curve adjuster
    */
    class StationaryHandle : public IAdjustmentHandle
    {
    public:
        StationaryHandle(float diameter, pointType pos);
        void paint(juce::Graphics&) override;
        void HandlePossibleMouseOver(const pointType mousePos) override;
        bool GetIsMouseWithin() override;
        pointType GetPos() override;
        void SetPos(pointType) override;
        float GetSize() override;
        bool GetCanMoveHorizontally() override;
        bool GetCanMoveVertically() override;
        void ForceMouseWithinFalse() override;
        void ForceMouseWithinTrue() override;
        void SetSelectedForMultiSelect(bool) override;
        bool GetIsSelectedInMultiSelect() override;

    private:
        void DetermineMouseEnter(const pointType mousePos) override;
        pointType OffsetPointForOrigin(pointType p) override;
        pointType GetCenterFromOrigin() override;

        const float size{ 10.0f };
        const float halfSize{ 5.0f };
        //const float mouseCushion{ 5.0f };
        const pointType pos{ 0.0f, 0.0f };
    };
}

/*
  ==============================================================================

    StationaryHandle.cpp
    Created: 20 Jan 2023 5:54:06pm
    Author:  thegr

  ==============================================================================
*/

#include "StationaryHandle.h"
namespace CurveAdjuster
{
    
    StationaryHandle::StationaryHandle(float diameter, pointType _pos)
    : size(diameter), halfSize(diameter / 2.0f), pos(OffsetPointForOrigin(_pos))
    {
    }
    void StationaryHandle::paint(juce::Graphics&)
    {
        //we don't need to see this.
    }
    void StationaryHandle::HandlePossibleMouseOver(const pointType)
    {
    }
    bool StationaryHandle::GetIsMouseWithin()
    {
        return false;
    }
    pointType StationaryHandle::GetPos()
    {
        return GetCenterFromOrigin();
    }
    void StationaryHandle::SetPos(pointType)
    {
    }
    float StationaryHandle::GetSize()
    {
        return size;
    }
    bool StationaryHandle::GetCanMoveHorizontally()
    {
        return false;
    }
    bool StationaryHandle::GetCanMoveVertically()
    {
        return false;
    }
    void StationaryHandle::ForceMouseWithinFalse()
    {
    }
    void StationaryHandle::ForceMouseWithinTrue()
    {
    }
    void StationaryHandle::SetSelectedForMultiSelect(bool)
    {
    }

    bool StationaryHandle::GetIsSelectedInMultiSelect()
    {
        return false;
    }
    
    void StationaryHandle::DetermineMouseEnter(const pointType)
    {
    }

    pointType StationaryHandle::OffsetPointForOrigin(pointType p)
    {
        //adjust mouse point to origin of handle
        return { p.x - halfSize, p.y - halfSize };
    }
    pointType StationaryHandle::GetCenterFromOrigin()
    {
        return { pos.x + halfSize, pos.y + halfSize };
    }
}

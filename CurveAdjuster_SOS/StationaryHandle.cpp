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
    : size(diameter), halfSize(diameter / 2.0f), pos(_pos)
    {
    }
    void StationaryHandle::paint(juce::Graphics&)
    {
    }
    void StationaryHandle::HandlePossibleMouseOver(const pointType mousePos)
    {
    }
    bool StationaryHandle::GetIsMouseWithin()
    {
        return mouseIsWithin;
    }
    pointType StationaryHandle::GetPos()
    {
        return GetCenterFromOrgin();
    }
    void StationaryHandle::SetPos(pointType)
    {
        // no need to move after initilazation!
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
        mouseIsWithin = false;
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
    
    void StationaryHandle::DetermineMouseEnter(const pointType mousePos)
    {
        auto xMin = pos.x - mouseCushion;
        auto xMax = pos.x + size + mouseCushion;
        auto yMin = pos.y - mouseCushion;
        auto yMax = pos.y + size + mouseCushion;
        if (mousePos.x >= xMin && mousePos.x <= xMax)
        {
            if (mousePos.y >= yMin && mousePos.y <= yMax)
            {
                mouseIsWithin = true;
                return;
            }
        }
        mouseIsWithin = false;
    }
    pointType StationaryHandle::OffsetPointForOrigin(pointType p)
    {
        //adjust mouse point to origin of handle
        return { p.x - halfSize, p.y - halfSize };
    }
    pointType StationaryHandle::GetCenterFromOrgin()
    {
        return { pos.x + halfSize, pos.y + halfSize };
    }
}

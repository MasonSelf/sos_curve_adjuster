/*
  ==============================================================================

    MovableHandleBase.cpp
    Created: 4 Apr 2023 1:21:44pm
    Author:  thegr

  ==============================================================================
*/


#pragma once
#include "MovableHandleBase.h"


namespace CurveAdjuster
{

    MovableHandleBase::MovableHandleBase(float diameter)
        :size(diameter), halfSize(diameter / 2.0f)
    {
    }

    MovableHandleBase::MovableHandleBase(float diameter, pointType p)
        : size(diameter), halfSize(diameter / 2.0f)
    {
        pos = OffsetPointForOrigin(p);
    }

    void MovableHandleBase::paint(juce::Graphics& g)
    {
        mouseIsWithin ? g.setColour(juce::Colours::red) : g.setColour(juce::Colours::white);
        if (isMultiSelected)
        {
            g.setColour(juce::Colours::maroon);
        }
        g.fillEllipse(pos.x, pos.y, size, size);
    }

    void MovableHandleBase::HandlePossibleMouseOver(const pointType mousePos)
    {
        bool previousState = mouseIsWithin;
        DetermineMouseEnter(mousePos);
        if (previousState != mouseIsWithin)
        {
            repaint();
        }
    }

    bool MovableHandleBase::GetIsMouseWithin()
    {
        return mouseIsWithin;
    }

    pointType MovableHandleBase::GetPos()
    {
        return GetCenterFromOrgin();
    }


    void MovableHandleBase::SetPos(pointType p)
    {
        pos = OffsetPointForOrigin(p);
        repaint();
    }

    float MovableHandleBase::GetSize()
    {
        return size;
    }

    bool MovableHandleBase::GetCanMoveHorizontally()
    {
        return false;
    }

    bool MovableHandleBase::GetCanMoveVertically()
    {
        return false;
    }

    void MovableHandleBase::ForceMouseWithinFalse()
    {
        mouseIsWithin = false;
        repaint();
    }

    void MovableHandleBase::ForceMouseWithinTrue()
    {
        mouseIsWithin = true;
        //don't repaint because this is currently only used when deleting a random handle within multi select
    }


    void MovableHandleBase::SetSelectedForMultiSelect(bool shouldBeSelected)
    {
        isMultiSelected = shouldBeSelected;
        repaint();
    }
    bool MovableHandleBase::GetIsSelectedInMultiSelect()
    {
        return isMultiSelected;
    }

    void MovableHandleBase::DetermineMouseEnter(const pointType mousePos)
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

    pointType MovableHandleBase::OffsetPointForOrigin(pointType p)
    {
        //adjust mouse point to origin of handle
        return { p.x - halfSize, p.y - halfSize };
    }

    pointType MovableHandleBase::GetCenterFromOrgin()
    {
        return { pos.x + halfSize, pos.y + halfSize };
    }

}

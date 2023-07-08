/*
  ==============================================================================

    MultiSelectionManager.cpp
    Created: 26 Apr 2023 12:44:13pm
    Author:  Mason Self

  ==============================================================================
*/

#include "MultiSelectionManager.h"

MultiSelectManager::MultiSelectManager(float _handleSize, float maxX, float maxY)
    : handleSize(_handleSize), component_xMax(maxX), component_yMax(maxY), generator(rand())
{
}

MultiSelectManager::~MultiSelectManager()
{
    if (isTimerRunning())
    {
        stopTimer();
    }
}

void MultiSelectManager::paint(juce::Graphics& g)
{
    if (selectionInProgress)
    {
        g.setColour(juce::Colours::grey);
        g.setOpacity(0.5f);
        if (counterForTimer >= ((counterMax / 2)))
        {
            g.setOpacity(static_cast<float>(counterForTimer - counterMax) * -1.0f / static_cast<float>(counterMax));
        }
        g.fillRect(selectionRectangle);
    }
}

void MultiSelectManager::timerCallback()
{
    ++counterForTimer;

    if (counterForTimer == counterMax)
    {
        selectionInProgress = false;
        counterForTimer = 0;
        stopTimer();
    }
     if (! selectionInProgress)
    {
        counterForTimer = 0;
        stopTimer();
    }
        repaint();

}

bool MultiSelectManager::IsPointWithinMultiSection(const pointType& p)
{
    DetermineSelectionMinAndMax();
    return p.x > xMin && p.x < xMax        ?
                p.y > yMin && p.y < yMax ?
                    true : false
                                             : false;
}

void MultiSelectManager::SetTranslation(const pointType& p)
{
    axisTranslations = {p.x - selectionOriginalPos.x, p.y - selectionOriginalPos.y};
}

pointType MultiSelectManager::TranslatePoint(const pointType& p)
{
    return {p.x + axisTranslations.x, p.y + axisTranslations.y};
}

void MultiSelectManager::SetSelectionStart(const pointType& p)
{
    selectionStartPoint = p;
}

void MultiSelectManager::SetSelectionEnd(const pointType& p)
{
    selectionEndPoint = p;
    selectionRectangle = {selectionStartPoint, selectionEndPoint};
    startTimerHz(timerHz);
    repaint();
}

void MultiSelectManager::MoveSelection()
{
    counterForTimer = 0;
    
    selectionRectangle.translate(axisTranslations.x, axisTranslations.y);
    selectionStartPoint = TranslatePoint(selectionStartPoint);
    selectionEndPoint = TranslatePoint(selectionEndPoint);
    repaint();

}

//bool MultiSelectManager::MoveIsWithinLimits()
//{
//    if (lowerX_Lim.reached || upperX_Lim.reached || lowerY_Lim.reached || upperY_Lim.reached)
//    {
//        return false;
//    }
//    return true;
//}


void MultiSelectManager::SetAndLimitTranslation(const handleCollectionIterator& begin, const handleCollectionIterator& end, const pointType& mousePos)
{
    SetTranslation(mousePos);
    //limit translation if need be
    for (auto it = begin; it != end; ++it)
    {
        if ((*it)->GetIsSelectedInMultiSelect())
        {
            LimitXTranslation(begin, end, it);
            LimitYTranslation(begin, end, it);
        }
    }
    //DebugHelp::PrintPoint(axisTranslations);

}

void MultiSelectManager::DetermineHandlesInSelection(const handleCollectionIterator& begin, const handleCollectionIterator& end)
{
    for (auto it = begin; it != end; ++it)
    {
        (*it)->SetSelectedForMultiSelect(IsPointWithinMultiSection((*it)->GetPos()));
        (*it)->repaint();
    }
}

bool MultiSelectManager::GetRandomPointWithinSelection(const handleCollectionIterator& begin, const handleCollectionIterator& end, pointType& result)
{
    DetermineSelectionMinAndMax();
    std::uniform_real_distribution<> xDistribution(xMin + handleSize, xMax - handleSize);
    std::uniform_real_distribution<> yDistribution(yMin + handleSize, yMax - handleSize);

    pointType possiblePoint;

    for (int numAttempts = 0; numAttempts < 25; ++numAttempts)
    {
        possiblePoint = {static_cast<float>(xDistribution(generator)), static_cast<float>(yDistribution(generator))};
        
        bool isTooCloseToAnotherPoint {false};
        for (auto it = begin; it != end; ++it)
        {
            auto otherHandlePos = (*it)->GetPos();
            if (std::abs(otherHandlePos.x - possiblePoint.x) < (handleSize * 2.0f) && std::abs(otherHandlePos.y - possiblePoint.y) < (handleSize * 2.0f))
            {
                isTooCloseToAnotherPoint = true;
            }
            if (possiblePoint.x < handleSize || possiblePoint.x > (component_xMax - handleSize) ||
                     possiblePoint.y < handleSize || possiblePoint.y > (component_yMax - handleSize))
            {
                isTooCloseToAnotherPoint = true;
            }
        }
        if (! isTooCloseToAnotherPoint)
        {
            result = possiblePoint;
            return true;
        }
    }
    return false;
}

std::pair<pointType, pointType> MultiSelectManager::GetMiddle_LR_Points()
{
    auto midY = selectionRectangle.getBottom() - (selectionRectangle.getHeight() / 2.0f);
    pointType left {selectionRectangle.getX(), midY};
    pointType right {selectionRectangle.getRight(), midY};
    return {left, right};
}

std::pair<pointType, pointType> MultiSelectManager::GetRampUpPoints()
{
    return {selectionRectangle.getBottomLeft(), selectionRectangle.getTopRight()};
}
std::pair<pointType, pointType> MultiSelectManager::GetRampDownPoints()
{
    return {selectionRectangle.getTopLeft(), selectionRectangle.getBottomRight()};
}

void MultiSelectManager::ResetTimer()
{
    counterForTimer = 0;
    if (! isTimerRunning())
    {
        startTimer(timerHz);
    }
}

void MultiSelectManager::ShortenTimer()
{
    counterForTimer = counterMax / 2;
}

void MultiSelectManager::LimitXTranslation(const handleCollectionIterator& begin, const handleCollectionIterator& end, const handleCollectionIterator& it)
{
    auto thisPoint = (*it)->GetPos();
    
    //if moving to left
    if (axisTranslations.x < 0.0f)
    {
        jassert(it != begin);
        //safe to decrement
        auto prev = it;
        --prev;
        //this only neccesary if neighbor is NOT part of the move
        if (! (*prev)->GetIsSelectedInMultiSelect())
        {
            auto leftNeighborPoint = (*prev)->GetPos();
            //greater restriction if y's are within handle size
            if (std::abs(leftNeighborPoint.y - thisPoint.y) <= handleSize * 2.0f)
            {
                if ((leftNeighborPoint.x + handleSize + 1.0f) >= TranslatePoint(thisPoint).x)
                {
                    axisTranslations.x = 0.0f;
                }
            }
            //special case if this is hits the left side
            else if (prev == begin)
            {
                if (TranslatePoint(thisPoint).x <= (handleSize / 2.0f))
                {
                    axisTranslations.x = 0.0f;
                }
            }
            //regular case (mid point, not close to neighbor)
            else if ((leftNeighborPoint.x + 1.0f) >= TranslatePoint(thisPoint).x)
            {
                //limit translation
                axisTranslations.x = 0.0f;
            }
        }

    }
    //if moving to right
    else if (axisTranslations.x > 0.0f)
    {
        jassert(it != end);
        //safe to increment
        auto next = it;
        ++next;
        //this is only neccesary if neighbor is NOT part of multiselect
        if (! (*next)->GetIsSelectedInMultiSelect())
        {
            jassert(next != end);
            //safe to increment
            auto possibleEnd = next;
            ++possibleEnd;
            auto rightNeighborPoint = (*next)->GetPos();
            //most restriction if y's are close
            if (std::abs(rightNeighborPoint.y - thisPoint.y) <= handleSize * 2.0f)
            {
                if ((rightNeighborPoint.x - handleSize - 1.0f) <= TranslatePoint(thisPoint).x)
                {
                    axisTranslations.x = 0.0f;
                }
            }
            //special scenario if second to last handle hitting right wall
            else if (possibleEnd == end)
            {
                if (TranslatePoint(thisPoint).x >= (component_xMax - handleSize / 2.0f))
                {
                    axisTranslations.x = 0.0f;
                }
            }
            //regular case (mid point,neighbor's y not close}
            else if ((rightNeighborPoint.x - 1.0f) <= TranslatePoint(thisPoint).x)
            {
                axisTranslations.x = 0.0f;
            }
        }
    }
}

void MultiSelectManager::LimitYTranslation(const handleCollectionIterator& begin, const handleCollectionIterator& end, const handleCollectionIterator& it)
{
    auto thisPoint = (*it)->GetPos();
    
    //left neighbor
    jassert(it != begin);
    //safe to decrement
    auto prev = it;
    --prev;
    if (! (*prev)->GetIsSelectedInMultiSelect())
    {
        auto prevPoint = (*prev)->GetPos();
        
        if (std::abs(prevPoint.x - thisPoint.x) <= handleSize)
        {
            //y translation is negative (up)
            if (axisTranslations.y < 0.0f)
            {
                if ((prevPoint.y + handleSize + 1.0f) >= TranslatePoint(thisPoint).y)
                {
                    axisTranslations.y = 0.0f;
                }
            }
            //y translation is postive (down)
            if (axisTranslations.y > 0.0f)
            {
                if ((prevPoint.y - handleSize - 1.0f) <= TranslatePoint(thisPoint).y)
                {
                    axisTranslations.y = 0.0f;
                }
            }
        }
    }
    //right neighbor
    if (axisTranslations.y != 0.0f) //only neccesary if y translation hasn't yet been axed
    {
        auto next = it;
        ++next;
        jassert(next != end); //safe to dereference
        if (! (*next)->GetIsSelectedInMultiSelect())
        {
            auto nextPoint = (*next)->GetPos();
            
            if (std::abs(nextPoint.x - thisPoint.x ) <= handleSize)
            {
                //y translate negative (up)
                if (axisTranslations.y < 0.0f)
                {
                    if ((nextPoint.y + handleSize + 1.0f) >= TranslatePoint(thisPoint).y)
                    {
                        axisTranslations.y = 0.0f;
                    }
                }
                //y translate positive (down)
                if (axisTranslations.y > 0.0f)
                {
                    if ((nextPoint.y - handleSize - 1.0f) <= TranslatePoint(thisPoint).y)
                    {
                        axisTranslations.y = 0.0f;
                    }
                }
            }
        }
    }
    
    //general limit top and bottom
    if (axisTranslations.y < 0.0f && TranslatePoint(thisPoint).y < 0.0f)
    {
        axisTranslations.y = 0.0f;
    }
    else if (axisTranslations.y > 0.0f && TranslatePoint(thisPoint).y > component_yMax)
    {
        axisTranslations.y = 0.0f;
    }
}

void MultiSelectManager::DetermineSelectionMinAndMax()
{
    if (selectionStartPoint.x < selectionEndPoint.x)
    {
        xMin = selectionStartPoint.x;
        xMax = selectionEndPoint.x;
    }
    else
    {
        xMax = selectionStartPoint.x;
        xMin = selectionEndPoint.x;
    }
    
    if (selectionStartPoint.y < selectionEndPoint.y)
    {
        yMin = selectionStartPoint.y;
        yMax = selectionEndPoint.y;
    }
    else
    {
        yMax = selectionStartPoint.y;
        yMin = selectionEndPoint.y;
    }
}

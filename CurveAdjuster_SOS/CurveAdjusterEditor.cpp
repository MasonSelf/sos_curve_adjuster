/*
  ==============================================================================

    CurveAdjusterEditor.cpp
    Created: 19 Jan 2023 9:30:24pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include "CurveAdjusterEditor.h"


using namespace CurveAdjuster;

CurveAdjusterEditor::CurveAdjusterEditor(float _width, 
    float _height, CurveAdjusterProcessor& p, bool _minIsAdjustable, bool _maxIsAdjustable, bool receivesModulation)
: curveAdjusterProcessor(p), width(_width), height(_height), minIsAdjustable(_minIsAdjustable), maxIsAdjustable(_maxIsAdjustable), multiSelectManager(handleSize, _width, _height)
{
    handleChanged.setValue(false);
    handleChanged.addListener(this);
    replacementHappened.setValue(false);
    replacementHappened.addListener(this);
    paramValue.addListener(this);
    
    if (! p.ReadyForGuiInit.load())
    {
        startTimer(timerIDs::initTimer, timerIntervalMs);
    }
    
    if (receivesModulation)
    {
        startTimer(timerIDs::drawTracesUnderModulation, timerIntervalMs);
    }
    else
    {
        startTimer(timerIDs::drawTracesDuringChange, timerIntervalMs);
    }
    
    InitHandles();
    addAndMakeVisible(multiSelectManager);
    setWantsKeyboardFocus(true);
}

CurveAdjusterEditor::~CurveAdjusterEditor()
{
    handleChanged.removeListener(this);
    replacementHappened.removeListener(this);
    if (isTimerRunning(timerIDs::initTimer))
    {
        stopTimer(timerIDs::initTimer);
    }
    if (isTimerRunning(timerIDs::drawTracesDuringChange))
    {
        stopTimer(timerIDs::drawTracesDuringChange);
    }
    if (isTimerRunning(timerIDs::drawTracesUnderModulation))
    {
        stopTimer(timerIDs::drawTracesUnderModulation);
    }
}

void CurveAdjusterEditor::resized()
{    
    for (auto& h : handles)
    {
        h->setBounds(0, 0, getWidth(), getHeight());
    }
    for (auto& c : connectors)
    {
        c.setBounds(0, 0, getWidth(), getHeight());
    }
    multiSelectManager.setBounds(0, 0, getWidth(), getHeight());
}

void CurveAdjusterEditor::paint(juce::Graphics& g)
{
    using namespace juce::Colours;
    g.fillAll(black);
    
    //draw reference lines to associated parameter value
    if (paramRecentlyChanged)
    {
        g.setColour(grey);

        float counterF = counterForDrawingTraces;
        float maxF = counterForDrawingTracesMax;
        g.setOpacity((maxF - counterF) / maxF * 0.8f);
        
        auto x = curveAdjusterProcessor.inputX.get() * (width - 1);
        auto yIntersect = GetY_AtX(x);
        g.drawVerticalLine(static_cast<int>(x), yIntersect, height);
        g.drawHorizontalLine(static_cast<int>(yIntersect), x, width);
    }

}

void CurveAdjusterEditor::InitHandles()
{
    // see if it s empty
    //clear out initial values if there was a saved state calling this function again
    if (! handles.empty() )
    {
        handles.clear();
    }
    if (! connectors.empty())
    {
        connectors.clear();
    }
    for (size_t i = 0; i < curveAdjusterProcessor.data.maxConnectors.load(); ++i)
    {
        pointType start = GetCoordinateFromPercentage({curveAdjusterProcessor.data[i].startX.load(), curveAdjusterProcessor.data[i].startY.load()});
        pointType control = GetCoordinateFromPercentage({curveAdjusterProcessor.data[i].controlX.load(), curveAdjusterProcessor.data[i].controlY.load()});
        pointType end = GetCoordinateFromPercentage({curveAdjusterProcessor.data[i].endX.load(), curveAdjusterProcessor.data[i].endY.load()});
        
        AddHandle(start);
        
        AddHandleConnection(start, control , end, connectors.end());
        if (curveAdjusterProcessor.data[i].endX.load() == 1.0f)
        {
            AddHandle(end);
            return;
        }
    }
    
    jassertfalse; //there should always be an end point!
}

void CurveAdjusterEditor::mouseEnter(const juce::MouseEvent& e)
{
    mouseMove(e);
}

void CurveAdjusterEditor::mouseDown(const juce::MouseEvent& e)
{
    if (multiSelectManager.selectionInProgress)
    {
        //end multi selection if position outside of multi selection
        if (! multiSelectManager.IsPointWithinMultiSection(e.position))
        {
            multiSelectManager.selectionInProgress = false;
            multiSelectManager.moveInProgress = false;
            for (auto& h : handles)
            {
                h->SetSelectedForMultiSelect(false);
            }
            return;
        }
        else if (e.mods.isRightButtonDown())
        {
            HandleRightClickOptionsInMultiSelect();
        }
        //otherwise signal that selection might be about to move
        else
        {
            multiSelectManager.moveInProgress = true;
            multiSelectManager.selectionOriginalPos = e.position;
        }
    }
    else if (e.mods.isRightButtonDown())
    {
        HandleRightClickOptionsNoMultiSelect();
    }
    if (e.mods.getCurrentModifiers().isCommandDown())
    {
        mouseDoubleClick(e);
    }
}

void CurveAdjusterEditor::mouseUp(const juce::MouseEvent&)
{
    if (multiSelectManager.moveInProgress)
    {
        multiSelectManager.moveInProgress = false;
        undoManager.AddState(connectors);
    }
    
    if (mouseDragChangeInProgress)
    {
        mouseDragChangeInProgress = false;
        undoManager.AddState(connectors);
    }
}

void CurveAdjusterEditor::mouseMove(const juce::MouseEvent& e)
{
    if (DetermineMouseOverHandles(e.position))
    {
        for (auto& c : connectors)
        {
            c.ForceMouseOverFalse();
        }
    }
    else
    {
        DetermineMouseOverConnectors(e.position);
    }
}

void CurveAdjusterEditor::mouseExit(const juce::MouseEvent&)
{
    for (auto it = handles.begin(); it != handles.end(); ++it)
    {
        if ((*it)->GetIsMouseWithin())
        {
            (*it)->ForceMouseWithinFalse();
        }
    }
    for (auto& c : connectors)
    {
        if (c.mouseOver)
        {
            c.ForceMouseOverFalse();
        }
    }
    if (multiSelectManager.selectionInProgress)
    {

        if (! multiSelectManager.isTimerRunning())
        {
            multiSelectManager.ResetTimer();
        }
    }
}

void CurveAdjusterEditor::mouseDrag(const juce::MouseEvent& event)
{
    HandleMouseDragWithPosition(GetPointTypeFromMousePos(event));
}

//This makes it easier to illicit behavior with only a position
void CurveAdjusterEditor::HandleMouseDragWithPosition(pointType pos)
{
    //case: multi selection move
    if (multiSelectManager.moveInProgress)
    {
        multiSelectManager.ResetTimer();
        if (! isTimerRunning(timerIDs::timeOutForMultiSelect))
        {
            startTimer(timerIDs::timeOutForMultiSelect, timerIntervalMs);
        }
        
        multiSelectManager.SetAndLimitTranslation(handles.begin(), handles.end(), pos);
        multiSelectManager.MoveSelection();
        
        //move points and associated connectors
        int index = 0;
        for (auto it = handles.begin(); it != handles.end(); ++it, ++index)
        {
            if ((*it)->GetIsSelectedInMultiSelect())
            {
                pointType newPos = multiSelectManager.TranslatePoint((*it)->GetPos());
                newPos = Move2DHandle(newPos, it, index, true);
                (*it)->SetPos(newPos);
                handleChanged.setValue(true);
            }
        }
        multiSelectManager.selectionOriginalPos = multiSelectManager.TranslatePoint(multiSelectManager.selectionOriginalPos);
    }
    
    //case: individual component move
    else
    {
        bool somethingWasSelected = false;
        
        //check if handle is selected
        int index = 0;
        for (auto it = handles.begin(); it != handles.end(); ++it, ++index)
        {
            if ((*it)->GetIsMouseWithin())
            {
                somethingWasSelected = true;
                mouseDragChangeInProgress = true;
                pointType p;
                if ((*it)->GetCanMoveHorizontally() && (*it)->GetCanMoveVertically())
                {
                    p = Move2DHandle(pos, it, index, false);
                }
                else if ((*it)->GetCanMoveVertically())
                {
                    p = RestrictPosition1D(pos, it);
                    
                    auto connectorsIt = connectors.begin();
                    for (int i = 0; i < index; ++i)
                    {
                        ++connectorsIt;
                    }
                    if (it == handles.begin())
                    {
                        auto next = it;
                        ++next;
                        RedrawConnectorsAfterHandleMoved(connectorsIt, p, (*next)->GetPos());
                    }
                    if (it == --handles.end())
                    {
                        auto previous = it;
                        --previous;
                        --connectorsIt;
                        RedrawConnectorsAfterHandleMoved(connectorsIt, (*previous)->GetPos(), p);
                    }
                }
                (*it)->SetPos(p);
                handleChanged.setValue(true);
            }
        }
        
        //check if connector highlighted
        if (! somethingWasSelected)
        {
            for (auto& c : connectors)
            {
                if (c.mouseOver)
                {
                    somethingWasSelected = true;
                    mouseDragChangeInProgress = true;
                    c.AdjustControlPoint(pos);
                    handleChanged.setValue(true);
                }
            }
        }
        
        //otherwise muliSelect
        if (! somethingWasSelected)
        {
            if (! multiSelectManager.selectionInProgress)
            {
                multiSelectManager.SetSelectionStart(pos);
                multiSelectManager.selectionInProgress = true;
            }
            else
            {
                multiSelectManager.SetSelectionEnd(pos);
                multiSelectManager.ResetTimer();
                if (! isTimerRunning(timerIDs::timeOutForMultiSelect))
                {
                    startTimer(timerIDs::timeOutForMultiSelect, timerIntervalMs);
                }
                multiSelectManager.DetermineHandlesInSelection(handles.begin(), handles.end());
            }
        }
    }
}

void CurveAdjusterEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    HandleMouseDoubleClickWithPostion(GetPointTypeFromMousePos(event));
    if (multiSelectManager.selectionInProgress)
    {
        multiSelectManager.selectionInProgress = false;
        multiSelectManager.moveInProgress = false;
    }
    undoManager.AddState(connectors);
}

void CurveAdjusterEditor::HandleMouseDoubleClickWithPostion(pointType pos)
{
    auto connectorsIt = connectors.begin();
    for (auto it = handles.begin() ; it != handles.end() ; ++it, ++connectorsIt)
    {
        if ((*it)->GetIsMouseWithin() )
        {
            if ((*it)->GetCanMoveHorizontally())
            {
                auto previousHandleIt = it;
                --previousHandleIt;
                auto nextHandleIt = it;
                ++nextHandleIt;
                RedrawConnectorsAfterHandleRemoved(connectorsIt, (*previousHandleIt)->GetPos(), (*nextHandleIt)->GetPos());
                
                //remove handle itself
                RemoveHandle(it);
                handleChanged.setValue(true);
                
                return;
            }
            else //ignore handle over end point
            {
                return;
            }
        }
    }
    
    if (AddHandle(pos))
    {
        handleChanged.setValue(true);
    }
}

pointType CurveAdjusterEditor::GetPointTypeFromMousePos(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();
    return {static_cast<float>(pos.x), static_cast<float>(pos.y)};
}

void CurveAdjusterEditor::valueChanged(juce::Value& value)
{
    
    if ((bool)(handleChanged.getValue()) == true || (bool)replacementHappened.getValue() == true)
    {
        //set last connector first! this way there is always an end
        int index = static_cast<int>(connectors.size()) - 1;
        for (auto it = connectors.rbegin(); it != connectors.rend(); ++it, --index)
        {
            auto start = GetPointAsPercentage(it->start);
            curveAdjusterProcessor.data[index].startX.store(start.x);
            curveAdjusterProcessor.data[index].startY.store(start.y);
            
            auto control = GetPointAsPercentage(it->control);
            curveAdjusterProcessor.data[index].controlX.store(control.x);
            curveAdjusterProcessor.data[index].controlY.store(control.y);
            
            auto end = GetPointAsPercentage(it->end);
            curveAdjusterProcessor.data[index].endX.store(end.x);
            curveAdjusterProcessor.data[index].endY.store(end.y);
        }
        
        //clear rest of data set
        auto remainderIndex = static_cast<size_t>(connectors.size());
        while (remainderIndex < curveAdjusterProcessor.data.maxConnectors.load())
        {
            curveAdjusterProcessor.data[remainderIndex].startX.store(-1.0f);
            curveAdjusterProcessor.data[remainderIndex].startY.store(-1.0f);
            curveAdjusterProcessor.data[remainderIndex].controlX.store(-1.0f);
            curveAdjusterProcessor.data[remainderIndex].controlY.store(-1.0f);
            curveAdjusterProcessor.data[remainderIndex].endX.store(-1.0f);
            curveAdjusterProcessor.data[remainderIndex].endY.store(-1.0f);
            ++remainderIndex;
        }
        
        if ((bool)handleChanged.getValue() == true)
        {
            handleChanged.setValue(false);
        }
        if ((bool)replacementHappened.getValue() == true)
        {
            replacementHappened.setValue(false);
        }
    
        //for generating presets more easily
        //PrintControlPoints();
    }
    
    //associated parameter changed
    if (value == paramValue)
    {
        if (isTimerRunning(CurveAdjusterEditor::timerIDs::drawTracesDuringChange))
        {
            counterForDrawingTraces = 0; //lengthen timer while change is happening
        }
        else
        {
            startTimer(CurveAdjusterEditor::timerIDs::drawTracesDuringChange, 30);
        }
    }
}

void CurveAdjusterEditor::timerCallback(int timerID)
{
    if (timerID == timerIDs::initTimer)
    {
        if (curveAdjusterProcessor.ReadyForGuiInit.load())
        {
            curveAdjusterProcessor.ReadyForGuiInit.store(false);
            stopTimer(timerIDs::initTimer);
            InitHandles();
        }
    }
    if (timerID == timerIDs::drawTracesDuringChange)
    {
        if (! paramRecentlyChanged)
        {
            paramRecentlyChanged = true;
        }
        ++counterForDrawingTraces;
        if (counterForDrawingTraces == counterForDrawingTracesMax)
        {
            paramRecentlyChanged = false;
            counterForDrawingTraces = 0;
            stopTimer(timerIDs::drawTracesDuringChange);
        }
        repaint();
    }
    if (timerID == timerIDs::timeOutForMultiSelect)
    {
        if (! multiSelectManager.selectionInProgress)
        {
            for (auto& h : handles)
            {
                if (h->GetIsSelectedInMultiSelect())
                {
                    h->SetSelectedForMultiSelect(false);
                }
            }
            repaint();
            stopTimer(timerIDs::timeOutForMultiSelect);
        }
    }
    if (timerID == timerIDs::drawTracesUnderModulation)
    {
        if (! juce::approximatelyEqual(curveAdjusterProcessor.inputX.get(), cachedValue))
        {
            cachedValue = curveAdjusterProcessor.inputX.get();
            if (! isTimerRunning(timerIDs::drawTracesDuringChange))
            {
                startTimer(timerIDs::drawTracesDuringChange, timerIntervalMs);
            }
            else
            {
                counterForDrawingTraces = 0;
            }
        }
    }
}


bool CurveAdjusterEditor::keyStateChanged(bool isKeyDown)
{
    if (!isKeyDown)
    {
        return true;
    }
    if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey)
        || juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::returnKey)
        || juce::KeyPress::isKeyCurrentlyDown('x')
        || juce::KeyPress::isKeyCurrentlyDown('t'))
    {
        return false;
    }
    if (juce::KeyPress::isKeyCurrentlyDown('s') && cmdDown)
    {
        return false;
    }
    if (juce::KeyPress::isKeyCurrentlyDown('a') && !cmdDown)
    {
        return false;
    }
    if (multiSelectManager.selectionInProgress)
    {
        if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::backspaceKey) || juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::deleteKey))
        {
            RemoveHandlesInMultiSelection();
            multiSelectManager.selectionInProgress = false;
            undoManager.AddState(connectors);
            return true;
        }
        
        if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::upKey) ||
            juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::downKey) ||
            juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey) ||
            juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey))
        {
            //simulate mouse drag up one pixel
            auto center = multiSelectManager.selectionRectangle.getCentre();
            multiSelectManager.selectionOriginalPos = center;
            if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::upKey))
            {
                center.y -= 1.0f;
            }
            else if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::downKey))
            {
                center.y += 1.0f;
            }
            else if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey))
            {
                center.x -= 1.0f;
            }
            else
            {
                center.x += 1.0f;
            }
            multiSelectManager.moveInProgress = true;
            HandleMouseDragWithPosition(center);
            multiSelectManager.moveInProgress = false;
            undoManager.AddState(connectors);
        }
    }
    if (cmdDown)
    {
        //undo and redo
        if (juce::KeyPress::isKeyCurrentlyDown('z'))
        {
            if (shiftDown)
            {
                auto& nextStateConnectors = undoManager.GetNextState();
                if (! nextStateConnectors.has_value())
                {
                    return true;
                }
                ReplaceStateFromUndoManagerConnectors(nextStateConnectors.value());
                return true;
            }
            auto& prevStateConnectors = undoManager.GetPreviousState();
            if (! prevStateConnectors.has_value())
            {
                return true;
            }
            ReplaceStateFromUndoManagerConnectors(prevStateConnectors.value());
            return true;
        }
        
        if (juce::KeyPress::isKeyCurrentlyDown('a')) //select all
        {
            multiSelectManager.SetSelectionStart({0.0f, 0.0f});
            multiSelectManager.SetSelectionEnd({width, height});
            multiSelectManager.selectionInProgress = true;
            multiSelectManager.ResetTimer();
            if (! isTimerRunning(timerIDs::timeOutForMultiSelect))
            {
                startTimer(timerIDs::timeOutForMultiSelect, timerIntervalMs);
            }
            multiSelectManager.DetermineHandlesInSelection(handles.begin(), handles.end());
        }
    }

    return true;
}

void CurveAdjusterEditor::modifierKeysChanged(const juce::ModifierKeys& modifiers)
{
    cmdDown = modifiers.isCommandDown();
    shiftDown = modifiers.isShiftDown();
}

bool CurveAdjusterEditor::DetermineMouseOverHandles(const pointType p)
{
    std::vector<handleCollectionIterator> selectedHandles;
    for (auto it = handles.begin(); it != handles.end(); ++it)
    {
        (*it)->HandlePossibleMouseOver(p);
        if ((*it)->GetIsMouseWithin())
        {
            selectedHandles.push_back(it);
        }
    }
    if (selectedHandles.empty())
    {
        return false;
    }
    if (selectedHandles.size() == 1)
    {
        return true;
    }
    //select only closest handle if multiple are selected
    if (selectedHandles.size() > 1)
    {
        auto closestHandle = selectedHandles.front();
        for (auto& handleIt : selectedHandles)
        {
            if (handleIt == closestHandle)
            {
                continue;
            }
            if ((*handleIt)->GetPos().getDistanceFrom(p) < (*closestHandle)->GetPos().getDistanceFrom(p))
            {
                //deselect old closest handle
                (*closestHandle)->ForceMouseWithinFalse();
                //make currently iterator the new closest handle
                closestHandle = handleIt;
            }
            //current iterator is NOT closer, so deselect it
            else
            {
                (*handleIt)->ForceMouseWithinFalse();
            }
        }
    }
    return true;
}

float CurveAdjusterEditor::GetWidth()
{
    return width;
}

float CurveAdjusterEditor::GetHeight()
{
    return height;
}

juce::Point<float> CurveAdjusterEditor::GetHandlePos(handleCollectionIterator it)
{
    return (*it)->GetPos();
}


void CurveAdjusterEditor::SetHandlePos(pointType p, handleCollectionIterator it)
{
    (*it)->SetPos(p);
}

coordinateCollection CurveAdjusterEditor::GetHandleCoordinates()
{
    coordinateCollection points;
    for (auto it = handles.begin(); it != handles.end(); ++it)
    {
        points.push_back(GetPointAsPercentage(GetHandlePos(it)));
    }
    
    //DebugHelp::PrintVectorOfPoints(points);
    return points;
}

std::vector<pointType> CurveAdjusterEditor::GetControlPointCoordinates()
{
    std::vector<pointType> controlPoints;
    for (auto c : connectors)
    {
        controlPoints.push_back(GetPointAsPercentage(c.control));
    }
    return controlPoints;
}

bool CurveAdjusterEditor::AddHandle(pointType p)
{
    if (connectors.size() >= curveAdjusterProcessor.data.maxConnectors.load())
    {
        return false; //can't add any more points, so return false
    }

    if (juce::approximatelyEqual(p.x, 0.0f))
    {
        if (minIsAdjustable)
        {
            handles.insert(handles.begin(), std::make_unique<AdjustmentHandle1D>(handleSize, p));
        }
        else
        {
            handles.insert(handles.begin(), std::make_unique<StationaryHandle>(handleSize, p));
        }
        SetupNewHandleComponent(handles.begin());
        return true;
    }
    else if (juce::approximatelyEqual(p.x, GetWidth()))
    {
        if (maxIsAdjustable)
        {
            handles.emplace_back(std::make_unique<AdjustmentHandle1D>(handleSize, p));
        }
        else
        {
            handles.emplace_back(std::make_unique<StationaryHandle>(handleSize, p));
        }
        auto lastElementIter = handles.end();
        --lastElementIter;
        SetupNewHandleComponent(lastElementIter);
        return true;
    }
    else if (handles.size() == 0)
    {
        handles.emplace_back(std::make_unique<AdjustmentHandle2D>(handleSize, p));
        
        SetupNewHandleComponent(handles.begin());
        return true;
    }
    
    else
    {
        //iterate to find appropriate insertion point to keep x's in order
        auto connectorsIt = connectors.begin();
        for (auto it = handles.begin(); it != handles.end(); ++it, ++connectorsIt)
        {
            //this shouldn't happen, is it neccesary?
            //if new x is less than the first x
            if (p.x < (*it)->GetPos().x)
            {
                handles.insert(handles.begin(), std::make_unique<AdjustmentHandle2D>(handleSize, p));
                SetupNewHandleComponent(handles.begin());
                return true;
            }
            
            //special case if new x is equal to existing handle
            if (juce::approximatelyEqual(p.x, (*it)->GetPos().x))
            {
               //consider attempting to offset by 1 for a better user experience if this happens often
                return false;
            }
            
            //see if new x if greater
            if (p.x > (*it)->GetPos().x)
            {
                auto nextElement = it;
                ++nextElement;
                
                //if this was the final element, add to the end
                if (nextElement == handles.end())
                {
                    handles.emplace_back(std::make_unique<AdjustmentHandle2D>(handleSize, p));
                    auto newLastElementIter = handles.end();
                    --newLastElementIter;
                    SetupNewHandleComponent(newLastElementIter);
                    return true;
                }
                
                //confirm that following element is bigger
                if ((*nextElement)->GetPos().x > p.x)
                {
                    handles.insert(nextElement, std::make_unique<AdjustmentHandle2D>(handleSize, p));
                    auto newElement = nextElement;
                    --newElement; //navigates back to freshly inserted element
                    SetupNewHandleComponent(newElement);
                    
                    //redraw connectors
                    auto previousElement = newElement;
                    --previousElement;
                    RedrawConnectorsAfterHandleAdded(connectorsIt, (*previousElement)->GetPos(), (*newElement)->GetPos(), (*nextElement)->GetPos());
                    return true;
                }
            }
            
        }
    }
    return false;
}

void CurveAdjusterEditor::RemoveHandle(handleCollectionIterator it)
{
    handles.erase(it);
    repaint();
}

void CurveAdjusterEditor::SetParamValue(double v)
{
    paramValue.setValue(v * (width - 1));
}

double CurveAdjusterEditor::GetParamValue()
{
    return paramValue.getValue();
}

void CurveAdjusterEditor::ReplaceState(const Connector::connectorsCollection& c)
{
    if (! handles.empty())
    {
        handles.clear();
    }
    for (auto it = c.begin(); it != c.end(); ++it)
    {
        AddHandle(it->start);
        //check if last connector
        auto next = it;
        ++next;
        if (next == c.end())
        {
            AddHandle(it->end);
        }
    }
    
    //add paths between handles
    if (! connectors.empty())
    {
        connectors.clear();
    }
    if (handles.size() >= 2)
    {
        auto connectorsIt = c.begin();
        for (auto it = handles.begin(); it != handles.end(); ++it, ++connectorsIt)
        {
            auto next = it;
            ++next;
            if (next != handles.end())
            {
                if (connectorsIt != c.end())
                {
                    auto controlP = connectorsIt->control;
                    AddHandleConnection((*it)->GetPos(), controlP ,(*next)->GetPos(), connectors.end());
                }
            }
        }
    }
    replacementHappened.setValue(true);
    undoManager.AddState(connectors);
}

pointType CurveAdjusterEditor::Move2DHandle(const pointType& newPos, handleCollectionIterator it, int indexForConnector, bool hasAlreadyBeenRestricted)
{
    auto p = newPos;
    
    if (! hasAlreadyBeenRestricted)
    {
        p = RestrictPosition2D(newPos, it);
    }

    if (p.y < 0.0f)
    {
        p.y = 0.0f;
    }

    auto connectorsIt = connectors.begin();
    for (int i = 0; i < indexForConnector; ++i)
    {
        ++connectorsIt;
    }
    //assuming that a 2D handle is somewhere between the beginning and the end
    RedrawConnectorsAfterHandleMoved(connectorsIt, p);
    return p;
}

void CurveAdjusterEditor::SetupNewHandleComponent(handleCollectionIterator it)
{
    (*it)->setBounds(0, 0, getWidth(), getHeight());
    addAndMakeVisible(it->get());
    repaint();
}

juce::Point<float> CurveAdjusterEditor::RestrictPosition2D(pointType p, const handleCollectionIterator& it)
{
    jassert((*it)->GetCanMoveHorizontally() && (*it)->GetCanMoveVertically());
    
    //restrict y
    {
        float lowerYLimit = 0.0f /*+ halfHandleSize*/;
        float upperYLimit = GetHeight() /*- halfHandleSize*/;
        
        //add vertical restriction if this point is too close to another point
        
        auto thisX = (*it)->GetPos().x;
        auto thisY = (*it)->GetPos().y;
        for (auto localIterator = handles.begin(); localIterator != handles.end(); ++localIterator)
        {
            if (localIterator == it) continue;
            
            auto testPoint = (*localIterator)->GetPos();
            
            //if x's are within handle size apart
            if (IsLessThanOrEqual<float>(std::abs(thisX - testPoint.x), handleSize))
            {
                if (testPoint.y < thisY)
                {
                    lowerYLimit = testPoint.y + handleSize;
                }
                else
                {
                    upperYLimit = testPoint.y - handleSize;
                }
            }
        }

        if (p.y < lowerYLimit)
        {
            p.y = lowerYLimit;
        }
        else if (p.y > upperYLimit)
        {
            p.y = upperYLimit;
        }
    }
     //limit x
    {
        //continues to restrict horizonatal movement according to other points
        
        //initial boundaries as if there are no other points
        float lowerXLimit = halfHandleSize;
        float upperXLimit = GetWidth() - halfHandleSize;
        
        //update limits depending on neighbors
        
        auto previousIt = it;
        --previousIt;
         if ((*previousIt)->GetPos().x > lowerXLimit)
         {
             lowerXLimit = (*previousIt)->GetPos().x;
             //limit further if y's are within handle size
             if (IsLessThanOrEqual(std::abs((*previousIt)->GetPos().y - (*it)->GetPos().y), handleSize * 1.5f))
             {
                 lowerXLimit += handleSize;
             }
         }
        
        auto nextIt = it;
        ++nextIt;
         if ((*nextIt)->GetPos().x < upperXLimit)
         {
             upperXLimit = (*nextIt)->GetPos().x;
             //limit further if y's are within handle size apart
             if (IsLessThanOrEqual(std::abs((*nextIt)->GetPos().y - (*it)->GetPos().y), handleSize * 2.0f))
             {
                 upperXLimit-= handleSize;
             }
         }

        if (p.x < lowerXLimit)
        {
            p.x = lowerXLimit + 1;
        }
        else if (p.x > upperXLimit)
        {
            p.x = upperXLimit - 1;
        }
    }
    return p;
}

pointType CurveAdjusterEditor::RestrictPosition1D(pointType p, const handleCollectionIterator& it)
{
    jassert((*it)->GetCanMoveHorizontally() == false);
    
    //ignore x change
    p.x = (*it)->GetPos().x;
    
    //restrict y
    {
        float lowerYLimit = 0.0f ;
        float upperYLimit = GetHeight();
        
        //add vertical restriction if this point is too close to another point
        
        auto thisX = (*it)->GetPos().x;
        auto thisY = (*it)->GetPos().y;
        for (auto localIterator = handles.begin(); localIterator != handles.end(); ++localIterator)
        {
            if (localIterator == it) continue;
            
            auto testPoint = (*localIterator)->GetPos();
            
            //if x's are within handle size apart
            if (IsLessThanOrEqual(std::abs(thisX - testPoint.x), handleSize))
            {
                //if thisY is within handle size below
                if (IsLessThanOrEqual(testPoint.y, thisY) && IsLessThanOrEqual(thisY - testPoint.y, handleSize))
                {
                    lowerYLimit = testPoint.y + handleSize;
                }
                //if thisY is within handle size above
                else if (IsGreaterThanOrEqual(testPoint.y, thisY) && IsLessThanOrEqual(testPoint.y - thisY, handleSize))
                {
                    upperYLimit = testPoint.y - handleSize;
                }
                
            }
        }
        
        if (p.y < lowerYLimit)
        {
            p.y = lowerYLimit;
        }
        else if (p.y > upperYLimit)
        {
            p.y = upperYLimit;
        }
        
    }
    
    return p;
}

void CurveAdjusterEditor::DetermineMouseOverConnectors(const juce::Point<float> p)
{
    
    for (auto& c : connectors)
    {
        if (IsLessThanOrEqual(c.start.x, p.x) && IsGreaterThanOrEqual(c.end.x, p.x))
        {
            juce::Point<float> nearestPoint;
            auto distanceAlongPathToNearestPoint = c.path.getNearestPoint(p, nearestPoint);

            if (distanceAlongPathToNearestPoint < 12.0f || (c.path.getLength() - distanceAlongPathToNearestPoint) < 12.0f)
            {
                c.ForceMouseOverFalse();
            }
            else
            {
                juce::Line<float> tempLine {p, nearestPoint};
                if (IsLessThanOrEqual(tempLine.getLength(), 5.0f))
                {
                    c.mouseOver = true;
                    c.repaint();
                }
                else
                {
                    c.ForceMouseOverFalse();
                }
            }
            
        }
        else if (c.mouseOver == true)
        {
            c.ForceMouseOverFalse();
        }
    }
}


pointType CurveAdjusterEditor::GetPointAsPercentage(pointType p)
{
    //scale to percentage (0 - 1)
    p.x = (float)p.x / (float)GetWidth();
    p.y = (float)p.y / (float)GetHeight();
    
    //flip y value so that bottom represents 0 and top represents 1
    p.y = (p.y - 1.0f) * -1.0f;
    
    return p;
}

pointType CurveAdjusterEditor::GetCoordinateFromPercentage(pointType p)
{
    //scale to gui size
    p.x = static_cast<float>(p.x) * static_cast<float>(GetWidth());
    p.y = static_cast<float>(p.y) * static_cast<float>(GetHeight());
    
    //flip y values so that 0 is at the top
    p.y = (static_cast<float>(p.y) - static_cast<float>(GetHeight())) * -1.0f;
    
    return p;
}

 float CurveAdjusterEditor::GetY_AtX(float in_X)
{
    for (auto& c : connectors)
    {
        if (juce::approximatelyEqual(c.start.x, in_X))
        {
            return c.start.y;
        }
        else if (juce::approximatelyEqual(c.end.x, in_X))
        {
            return c.end.y;
        }
        if (c.start.x < in_X && c.end.x > in_X)
        {
            return c.GetY_AlongPath(in_X);
        }
    }
    //safety default, return 0.0f
    return 0.0f;
}

//todo / dream list, have connectors set their own control points to curve fit smoothly
void CurveAdjusterEditor::AddHandleConnection(pointType start, pointType end, Connector::connectorsCollectionIterator it)
{
    Connector newConnector{start, end};
    
    auto thisConnector = connectors.insert(it, newConnector);
    thisConnector->setBounds(0, 0, getWidth(), getHeight());
    addAndMakeVisible(*thisConnector);
    repaint();
}

//for adding connector when control point is known
void CurveAdjusterEditor::AddHandleConnection(pointType start, pointType control, pointType end, Connector::connectorsCollectionIterator it)
{
    auto thisConnector = connectors.insert(it, Connector(start, control, end));
    thisConnector->setBounds(0, 0, getWidth(), getHeight());
    addAndMakeVisible(*thisConnector);
    repaint();
}

void CurveAdjusterEditor::RedrawConnectorsAfterHandleRemoved(Connector::connectorsCollectionIterator it, pointType newStart, pointType newEnd)
{
    //this should be at least the second connector path
    jassert(it != connectors.begin());
    
    auto previousIt = it;
    --previousIt;

    previousIt = connectors.erase(previousIt);
    previousIt = connectors.erase(previousIt);
    AddHandleConnection(newStart, newEnd, previousIt);
}


//expects connector iterator AT connector path to remove and be replaced by two new ones
void CurveAdjusterEditor::RedrawConnectorsAfterHandleAdded(Connector::connectorsCollectionIterator it, pointType start, pointType middle, pointType end)
{
    it = connectors.erase(it);
    AddHandleConnection(start, middle, it);
    AddHandleConnection(middle, end, it);
}

//for redrawing a connector when a handle moved at either the beginning or the end
void CurveAdjusterEditor::RedrawConnectorsAfterHandleMoved(Connector::connectorsCollectionIterator it, pointType start, pointType end)
{
    it = connectors.erase(it);
    AddHandleConnection(start, end, it);
}

void CurveAdjusterEditor::RedrawConnectorsAfterHandleMoved(Connector::connectorsCollectionIterator it, pointType movedHandlePoint)
{
    //this should be at least the second connector path
    jassert(it != connectors.begin());
    
    auto previousIt = it;
    --previousIt;
    
    previousIt->AdjustEndPoint(movedHandlePoint);
    it->AdjustStartPoint(movedHandlePoint);
}

void CurveAdjusterEditor::ReplaceStateFromUndoManagerConnectors(Connector::connectorsCollection& _c)
{
    if (! handles.empty() )
    {
        handles.clear();
    }
    for (auto it = _c.begin(); it!= _c.end(); ++it)
    {
        AddHandle(it->start);
        auto next = it;
        ++next;
        if (next == _c.end())
        {
            AddHandle(it->end);
        }
    }
    if (! connectors.empty())
    {
        connectors.clear();
    }
    
    if (handles.size() >= 2)
    {
        auto connectorIt = _c.begin();
        for (auto it = handles.begin(); it != handles.end(); ++it, ++connectorIt)
        {
            auto next = it;
            ++next;
            if (next != handles.end())
            {
                auto lastConnectorIt = connectors.end();
                --lastConnectorIt;
                
                if (connectorIt != _c.end())
                {
                    AddHandleConnection((*it)->GetPos(), connectorIt->control, (*next)->GetPos(), connectors.end());
                }
            }
        }
    }
    handleChanged.setValue(true);
}

void CurveAdjusterEditor::HandleRightClickOptionsNoMultiSelect()
{
    juce::PopupMenu m;
    juce::PopupMenu flatLineSubmenu;
    flatLineSubmenu.addItem(1, "bottom");
    if (minIsAdjustable)
    {
        flatLineSubmenu.addItem(2, "middle");
        flatLineSubmenu.addItem(3, "top");
    }
    m.addSubMenu("flat line", flatLineSubmenu);
    juce::PopupMenu rampUpSubmenu;
    rampUpSubmenu.addItem(4, "linear");
    rampUpSubmenu.addItem(5, "exponential");
    rampUpSubmenu.addItem(6, "logarithmic");
    rampUpSubmenu.addItem(7, "staircase");
    rampUpSubmenu.addItem(8, "binary");
    m.addSubMenu("ramp up", rampUpSubmenu);

    if (minIsAdjustable)
    {
        juce::PopupMenu rampDownSubmenu;
        rampDownSubmenu.addItem(9, "linear");
        rampDownSubmenu.addItem(10, "exponential");
        rampDownSubmenu.addItem(11, "logarithmic");
        rampDownSubmenu.addItem(12, "staircase");
        rampDownSubmenu.addItem(13, "binary");
        m.addSubMenu("ramp down", rampDownSubmenu);
    }
    m.showMenuAsync(juce::PopupMenu::Options(),
                    [this] (int result)
    {

        if (result == 1) //flat line bottom
        {
            ReplaceState({{{0.0f, height},{width / 3.0f, height}, {width, height}}});
            handleChanged.setValue(true);
        }
        else if (result == 2) //flat line middle
        {
            auto y = height / 2.0f;
            ReplaceState({{{0.0f, y}, {width / 3.0f, y}, {width, y}}});
            handleChanged.setValue(true);
        }
        else if (result == 3) //flat line top
        {
            ReplaceState({{{0.0f, 0.0f}, {width / 3.0f, 0.0f}, {width, 0.0f}}});
            handleChanged.setValue(true);
        }
        else if (result == 4) //ramp up linear
        {
            ReplaceState({{{0.0f, height}, {width * 0.8f, height * 0.2f}, {width, 0.0f}}});
            handleChanged.setValue(true);
        }
        else if (result == 5) //ramp up exponential
        {
            ReplaceState({{{0.0f, height}, {width, height}, {width, 0.0f}}});
            handleChanged.setValue(true);
        }
        else if (result == 6) //ramp up log
        {
            ReplaceState({{{0.0f, height}, {0.0f, 0.0f}, {width, 0.0f}}});
            handleChanged.setValue(true);
        }
        else if (result == 7) //ramp up staircase
        {
            //start with linear ramp up
            ReplaceState({{{0.0f, height}, {width * 0.8f, height * 0.2f}, {width, 0.0f}}});
            float xIncr = width / 8.0f - 1.0f;
            float yIncr = height / 7.0f;
            auto thisX = xIncr;
            auto thisY = height;
            HandleMouseDoubleClickWithPostion({thisX, thisY});
            bool lastFlag {false};
            while (!lastFlag)
            {
                thisY -= yIncr;
                thisX += 1.0f;
                HandleMouseDoubleClickWithPostion({thisX, thisY});
                if (IsLessThanOrEqual(thisY, 0.0f))
                {
                    lastFlag = true;
                }
                else
                {
                    thisX += xIncr;
                    HandleMouseDoubleClickWithPostion({thisX, thisY});
                }
            }
        }
        else if (result == 8) //rampup binary
        {
            //start with linear ramp up
            ReplaceState({{{0.0f, height}, {width * 0.8f, height * 0.2f}, {width, 0.0f}}});
            HandleMouseDoubleClickWithPostion({width / 2.0f, height});
            HandleMouseDoubleClickWithPostion({width / 2.0f + 1.0f, 0.0f});
        }
        else if (result == 9) //ramp down linear
        {
            ReplaceState({{{0.0f, 0.0f}, {1.0f, 1.0f}, {width, height}}});
            handleChanged.setValue(true);
        }
        else if (result == 10) //ramp down exponential
        {
            ReplaceState({{{0.0f, 0.0f}, {0.0f, height}, {width, height}}});
            handleChanged.setValue(true);
        }
        else if (result == 11) // ramp down log
        {
            ReplaceState({{{0.0f, 0.0f}, {width, 0.0f}, {width, height}}});
            handleChanged.setValue(true);
        }
        else if (result == 12) // ramp down stair
        {
            //start with linear ramp down
            ReplaceState({{{0.0f, 0.0f}, {1.0f, 1.0f}, {width, height}}});
            float xIncr = width / 8.0f - 1.0f;
            float yIncr = height / 7.0f;
            auto thisX = xIncr;
            auto thisY = 0.0f;
            HandleMouseDoubleClickWithPostion({thisX, thisY});
            bool lastFlag {false};
            while (!lastFlag)
            {
                thisY += yIncr;
                thisX += 1.0f;
                HandleMouseDoubleClickWithPostion({thisX, thisY});
                if (IsGreaterThanOrEqual(thisY, height))
                {
                    lastFlag = true;
                }
                else
                {
                    thisX += xIncr;
                    HandleMouseDoubleClickWithPostion({thisX, thisY});
                }
            }
            
        }
        else if (result == 13) // ramp down binary
        {
            //start with linear ramp down
            ReplaceState({{{0.0f, 0.0f}, {1.0f, 1.0f}, {width, height}}});
            HandleMouseDoubleClickWithPostion({width / 2.0f, 0.0f});
            HandleMouseDoubleClickWithPostion({width / 2.0f + 1.0f, height});
        }
    });
}
void CurveAdjusterEditor::HandleRightClickOptionsInMultiSelect()
{
    multiSelectManager.stopTimer();

    juce::PopupMenu m;
    juce::PopupMenu randomSubmenu;
    randomSubmenu.addItem(1, "randomize selection");
    if (connectors.size() < curveAdjusterProcessor.data.maxConnectors.load())
    {
        randomSubmenu.addItem(2, "add handle");
    }
    randomSubmenu.addItem(3, "take away handle");
    
    juce::PopupMenu replaceWith;
    replaceWith.addItem(4, "flat line");
    replaceWith.addItem(5, "ramp up");
    replaceWith.addItem(6, "ramp down");
    
    m.addSubMenu("replace with", replaceWith);
    m.addSubMenu("random", randomSubmenu);
    m.showMenuAsync(juce::PopupMenu::Options(),
                    [this] (int result)
    {

        if (result == 0)
        {
            
        }
        else if (result == 1) //randomize selection
        {
            auto numHandles = GetNumHandlesInMultiSelect();
            if (numHandles == 0)
            {
                numHandles = 5;
            }
            else
            {
                RemoveHandlesInMultiSelection();
            }
            for (auto i = 0; i < numHandles; ++i)
            {
                pointType newPoint;
                if (multiSelectManager.GetRandomPointWithinSelection(handles.begin(), handles.end(), newPoint))
                {
                    HandleMouseDoubleClickWithPostion(newPoint);
                    multiSelectManager.DetermineHandlesInSelection(handles.begin(), handles.end());
                }
            }
            undoManager.AddState(connectors);
            handleChanged.setValue(true);
            
        }
        else if (result == 2) //add handle randomly
        {
            pointType newPoint;
            if (multiSelectManager.GetRandomPointWithinSelection(handles.begin(), handles.end(), newPoint))
            {
                HandleMouseDoubleClickWithPostion(newPoint);
                multiSelectManager.DetermineHandlesInSelection(handles.begin(), handles.end());
            }
            undoManager.AddState(connectors);
            handleChanged.setValue(true);
        }
        else if (result == 3) //take away handle randomly
        {
            //pick random handle within selection
            int numHandles = GetNumHandlesInMultiSelect();
            //special case if only one handle
            if (numHandles == 1)
            {
                for (auto& h : handles)
                {
                    if (h->GetIsSelectedInMultiSelect())
                    {
                        h->ForceMouseWithinTrue();
                        HandleMouseDoubleClickWithPostion(h->GetPos());
                        handleChanged.setValue(true);
                        return;
                    }
                }
            }
            
            juce::Random r;
            auto handleToRemove = r.nextInt(numHandles - 1);
            
            int indexForHandleToRemove = 0;
            for (auto& h : handles)
            {
                if (h->GetIsSelectedInMultiSelect())
                {
                    if (indexForHandleToRemove == handleToRemove)
                    {
                        h->ForceMouseWithinTrue();
                        HandleMouseDoubleClickWithPostion(h->GetPos());
                        undoManager.AddState(connectors);
                        handleChanged.setValue(true);
                        return;
                    }
                    ++indexForHandleToRemove;
                }
            }
            
        }
        else if (result == 4) //replace with flat line
        {
            RemoveHandlesWithinXRangeOfMultiSelect();

            auto startAndEnd = multiSelectManager.GetMiddle_LR_Points();
            HandleSelectionReplacement(startAndEnd);
            return;
        }
        if (result == 5) //replace with ramp up
        {
            RemoveHandlesWithinXRangeOfMultiSelect();
            auto startAndEnd = multiSelectManager.GetRampUpPoints();
            HandleSelectionReplacement(startAndEnd);
            return;
        }
        if (result == 6) //replace with ramp down
        {
            RemoveHandlesWithinXRangeOfMultiSelect();
            auto startAndEnd = multiSelectManager.GetRampDownPoints();
            HandleSelectionReplacement(startAndEnd);
            return;
        }
    });
    multiSelectManager.stopTimer();
}

void CurveAdjusterEditor::RemoveHandlesInMultiSelection()
{
    //get number of selected handles
    int numSelectedHandles = 0;
    for (auto& h : handles)
    {
        if (h->GetIsSelectedInMultiSelect())
        {
            ++numSelectedHandles;
        }
    }
    while (numSelectedHandles > 0)
    {
        auto connectorsIt = connectors.begin();
        for (auto it = handles.begin() ; it != handles.end() ; ++it, ++connectorsIt)
        {
            if ((*it)->GetIsSelectedInMultiSelect())
            {
                auto previousHandleIt = it;
                --previousHandleIt;
                auto nextHandleIt = it;
                ++nextHandleIt;
                RedrawConnectorsAfterHandleRemoved(connectorsIt, (*previousHandleIt)->GetPos(), (*nextHandleIt)->GetPos());
                
                //remove handle itself
                RemoveHandle(it);
                handleChanged.setValue(true);
                --numSelectedHandles;
                break;
            }
        }
    }
}

int CurveAdjusterEditor::GetNumHandlesInMultiSelect()
{
    int numHandles = 0;
    for (auto& h : handles)
    {
        if (h->GetIsSelectedInMultiSelect())
        {
            ++numHandles;
        }
    }
    return numHandles;
}

void CurveAdjusterEditor::RemoveHandlesWithinXRangeOfMultiSelect()
{
    multiSelectManager.DetermineSelectionMinAndMax();
    
    //remove any handles within x range not just within selection!
    for (auto& h : handles)
    {
        if (IsGreaterThanOrEqual(h->GetPos().x, multiSelectManager.xMin) && IsLessThanOrEqual(h->GetPos().x, multiSelectManager.xMax))
        {
            h->SetSelectedForMultiSelect(true);
        }
    }
    RemoveHandlesInMultiSelection();
}

void CurveAdjusterEditor::HandleSelectionReplacement(std::pair<pointType, pointType> p)
{
    bool shouldKeepSelectionActive = true;
    if (IsGreaterThanOrEqual(p.first.x, halfHandleSize))
    {
        HandleMouseDoubleClickWithPostion(p.first);
    }
    else
    {
        shouldKeepSelectionActive = false;
        p.first.setX(0.0f);
        handles.front()->ForceMouseWithinTrue();
        multiSelectManager.moveInProgress = false;
        multiSelectManager.selectionInProgress = false;
        HandleMouseDragWithPosition(p.first);
        handles.front()->ForceMouseWithinFalse();
    }
    if (IsLessThanOrEqual(p.second.x, width - halfHandleSize))
    {
        HandleMouseDoubleClickWithPostion(p.second);
    }
    else
    {
        shouldKeepSelectionActive = false;
        p.second.setX(width);
        handles.back()->ForceMouseWithinTrue();
        multiSelectManager.moveInProgress = false;
        multiSelectManager.selectionInProgress = false;
        HandleMouseDragWithPosition(p.second);
        
    }
    if (shouldKeepSelectionActive)
    {
        AddPairToNewlyReplacedSelection(p);
    }
}

void CurveAdjusterEditor::AddPairToNewlyReplacedSelection(const std::pair<pointType, pointType>& p)
{
    for (auto& h : handles)
    {
        if (h->GetPos() == p.first || h->GetPos() == p.second)
        {
            h->SetSelectedForMultiSelect(true);
        }
    }
}

void CurveAdjusterEditor::PrintControlPoints()
{
    DBG(curveAdjusterProcessor.GetName() << ":");
    for (auto& c : connectors)
    {
        DBG("{{" << DebugHelp::GetPointAsString(c.start) << "}, " <<
            "{" << DebugHelp::GetPointAsString(c.control) << "}, " <<
            "{" << DebugHelp::GetPointAsString(c.end) << "}}, ");
    }
}

/*
  ==============================================================================

    CurveAdjusterEditor.h
    Created: 19 Jan 2023 9:30:24pm
    Author:  thegr

  ==============================================================================
*/

#pragma once
#include "ICurveAdjusterEditor.h"
//#include "../PluginProcessor.h"
#include "CurveAdjusterProcessor.h"
#include "CurveAdjusterPointTypes.h"

#include "AdjusterHandle2D.h"
#include "AdjusterHandle1D.h"
#include "StationaryHandle.h"
#include "Connector.h"
#include "MultiSelectionManager.h"
#include "SOSUndoManager.h"
#include <sos_maths/sos_maths.h>

//testing purposes only
#include "DebugHelperFunctions.h"



using namespace CurveAdjuster;


class CurveAdjusterEditor : public ICurveAdjusterEditor, public juce::MultiTimer
{
public:
    CurveAdjusterEditor(float _width, float _height, CurveAdjusterProcessor& p, bool _minIsAdjustable, bool _maxIsAdjustable, bool receivesModulation);
    ~CurveAdjusterEditor() override;
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    void InitHandles() override;
    
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void HandleMouseDragWithPosition(pointType pos);
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void HandleMouseDoubleClickWithPostion(pointType pos);
    pointType GetPointTypeFromMousePos(const juce::MouseEvent& event);
    void valueChanged(juce::Value& value) override;
    void timerCallback(int timerID) override;
    bool keyStateChanged(bool isKeyDown) override;
    void modifierKeysChanged(const juce::ModifierKeys& modifiers) override;
    bool DetermineMouseOverHandles(const juce::Point<float> p);
    float GetWidth() override;
    float GetHeight() override;
    pointType GetHandlePos(handleCollectionIterator it) ;
    void SetHandlePos(pointType p, handleCollectionIterator it) ;
    
    /*returns vector of coordinates scaled as percentage (0.0f to 1.0f)
     treats bottom left as 0 and top right as 1*/
    coordinateCollection GetHandleCoordinates();
    
    /*returns vector of coordinates for CONTROL POINTS scaled as percentage (0.0f to 1.0f)
    treats bottom left as 0 and top right as 1 */
    std::vector<pointType> GetControlPointCoordinates();
    
    /*uses GUI relevant coordinates
    if initializing from processor coordinates, use GetCoordinateFromPercentage()
    returns true if added point successfully */
    bool AddHandle(pointType p);
    
    void RemoveHandle(handleCollectionIterator it);
    void SetParamValue(double);
    double GetParamValue();
    void ReplaceState(const Connector::connectorsCollection& c);
    
    //public so owning component can also listen in to potentially overwrite a preset
    //this is used to signal updating the processor thread
    juce::Value handleChanged;
    
    //this is used to signal the GUI to update from a replacement method but NOT to override / change the preset
    juce::Value replacementHappened {false};
    
    float GetY_AtX(float in_X);

private:
    enum timerIDs
    {
        initTimer, drawTracesDuringChange, timeOutForMultiSelect, drawTracesUnderModulation
    };
    const int timerIntervalMs {30};
    bool paramRecentlyChanged {true}; //this is used to signal drawing traces
    int counterForDrawingTraces {0};
    const int counterForDrawingTracesMax {50};
    float cachedValue{0.0f};     //this detmermines whether to consisently redraw traces (drawTracesUnderModulation), or use drawTracesDuringChange
    
    //for key commands
    bool cmdDown {false};
    bool shiftDown {false};

    CurveAdjusterProcessor& curveAdjusterProcessor;

    pointType Move2DHandle(const pointType& newPos, handleCollectionIterator it, int indexForConnector, bool hasAlreadyBeenRestricted);
    void SetupNewHandleComponent(handleCollectionIterator it);
    pointType RestrictPosition2D(pointType p, const handleCollectionIterator& it);
    pointType RestrictPosition1D(pointType p, const handleCollectionIterator& it);
    
    //concerning connectors
    void DetermineMouseOverConnectors(const juce::Point<float> p);
    void AddHandleConnection(pointType start, pointType end, Connector::connectorsCollectionIterator it);
    void AddHandleConnection(pointType start, pointType control, pointType end, Connector::connectorsCollectionIterator it);
    void RedrawConnectorsAfterHandleRemoved(Connector::connectorsCollectionIterator it, pointType newStart, pointType newEnd);
    void RedrawConnectorsAfterHandleAdded(Connector::connectorsCollectionIterator it, pointType start, pointType middle, pointType end);
    void RedrawConnectorsAfterHandleMoved(Connector::connectorsCollectionIterator it, pointType start, pointType end);
    void RedrawConnectorsAfterHandleMoved(Connector::connectorsCollectionIterator it, pointType movedHandlePoint);
    
    //this keeps track of the associated parameters value for drawing reference
    juce::Value paramValue;
    
    //used to translate handle positions as described by GetHandleCoordinates
    pointType GetPointAsPercentage(pointType p);
    //translate from processor coordinates to gui relevant coordinates
    pointType GetCoordinateFromPercentage(pointType p);


    handleCollection handles;                   //points
    Connector::connectorsCollection connectors; //lines connecting points/handles

    UndoManager<Connector::connectorsCollection> undoManager{50};
    void ReplaceStateFromUndoManagerConnectors(Connector::connectorsCollection& _c);
    
    bool mouseDragChangeInProgress{false};
    
    const float width;
    const float height;
    const float handleSize{ 8.0f };
    const float halfHandleSize{ 4.0f };
    
    bool minIsAdjustable, maxIsAdjustable;
    
    void HandleRightClickOptionsNoMultiSelect();
    
    MultiSelectManager multiSelectManager;
    void HandleRightClickOptionsInMultiSelect();
    void RemoveHandlesInMultiSelection();
    int GetNumHandlesInMultiSelect();
    void RemoveHandlesWithinXRangeOfMultiSelect();
    void HandleSelectionReplacement(std::pair<pointType, pointType> p);
    void AddPairToNewlyReplacedSelection(const std::pair<pointType, pointType>& p);
    
    //for creating presets
    void PrintControlPoints();
};


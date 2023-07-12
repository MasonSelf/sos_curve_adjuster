/*
  ==============================================================================

    MultiSelectionManager.h
    Created: 26 Apr 2023 12:44:13pm
    Author:  Mason Self

  ==============================================================================
*/

#pragma once
#include "IAdjusterHandle.h"
#include "DebugHelperFunctions.h"
#include <random>

using namespace CurveAdjuster;

class MultiSelectManager : public MouseIgnoringComponent , public juce::Timer
{
public:
    MultiSelectManager(float _handleSize, float maxX, float maxY);
    ~MultiSelectManager() override;
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    bool IsPointWithinMultiSection(const pointType& p);
    pointType TranslatePoint(const pointType& p);
    void SetSelectionStart(const pointType& p);
    void SetSelectionEnd(const pointType& p);
    void MoveSelection();
    void SetAndLimitTranslation(const handleCollectionIterator& begin, const handleCollectionIterator& end, const pointType& mousePos);
    void DetermineHandlesInSelection(const handleCollectionIterator& begin, const handleCollectionIterator& end);
    bool GetRandomPointWithinSelection(const handleCollectionIterator& begin, const handleCollectionIterator& end, pointType& result);
    std::pair<pointType, pointType> GetMiddle_LR_Points();
    std::pair<pointType, pointType> GetRampUpPoints();
    std::pair<pointType, pointType> GetRampDownPoints();
    void DetermineSelectionMinAndMax();
    
    void ResetTimer();
    void ShortenTimer();
    //for debug
    juce::Identifier iD;
    
    
    bool selectionInProgress {false};
    bool moveInProgress {false};
    
    pointType selectionOriginalPos;
    pointType selectionStartPoint;
    pointType selectionEndPoint;
    juce::Rectangle<float> selectionRectangle;
    float xMin, xMax, yMin, yMax;
    
private:
    void SetTranslation(const pointType& p);
    void LimitXTranslation(const handleCollectionIterator& begin, const handleCollectionIterator& end, const handleCollectionIterator& it);
    void LimitYTranslation(const handleCollectionIterator& begin, const handleCollectionIterator& end, const handleCollectionIterator& it);
    
    
    pointType axisTranslations;
    const float handleSize;
    const float component_xMax, component_yMax;

    
    std::random_device rand;
    std::mt19937 generator;
    
    int counterForTimer {0};
    const int counterMax{100};
    const int timerHz{30};
};

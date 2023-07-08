/*
  ==============================================================================

    CurveAdjusterProcessorData.h
    Created: 15 May 2023 11:59:32am
    Author:  Mason Self

  ==============================================================================
*/
#pragma once
#include "CurveAdjusterPointTypes.h"

namespace CurveAdjuster
{

    struct ConnectorPoints
    {
        pointType start, control, end;
    };
    struct AtomicConnector
    {
        std::atomic<float> startX {-1.0f};
        std::atomic<float> startY {-1.0f};
        std::atomic<float> controlX {-1.0f};
        std::atomic<float> controlY {-1.0f};
        std::atomic<float> endX {-1.0f};
        std::atomic<float> endY {-1.0f};
        
    };
    
    struct CurveAdjusterProcessorData
    {
        AtomicConnector connector0;
        AtomicConnector connector1;
        AtomicConnector connector2;
        AtomicConnector connector3;
        AtomicConnector connector4;
        AtomicConnector connector5;
        AtomicConnector connector6;
        AtomicConnector connector7;
        AtomicConnector connector8;
        AtomicConnector connector9;
        AtomicConnector connector10;
        AtomicConnector connector11;
        AtomicConnector connector12;
        AtomicConnector connector13;
        AtomicConnector connector14;
        AtomicConnector connector15;
        AtomicConnector connector16;
        AtomicConnector connector17;
        AtomicConnector connector18;
        AtomicConnector connector19;
        AtomicConnector connector20;
        AtomicConnector connector21;
        AtomicConnector connector22;
        AtomicConnector connector23;
        AtomicConnector connector24;
        AtomicConnector connector25;
        AtomicConnector connector26;
        AtomicConnector connector27;
        AtomicConnector connector28;
        AtomicConnector connector29;
        
        const std::atomic<int> maxConnectors{30};
        
        AtomicConnector& operator[](int index)
        {
            jassert(index < maxConnectors.load()); //index is out of range!
            
            switch(index)
            {
                case 0:
                    return connector0;
                case 1:
                    return connector1;
                case 2:
                    return connector2;
                case 3:
                    return connector3;
                case 4:
                    return connector4;
                case 5:
                    return connector5;
                case 6:
                    return connector6;
                case 7:
                    return connector7;
                case 8:
                    return connector8;
                case 9:
                    return connector9;
                case 10:
                    return connector10;
                case 11:
                    return connector11;
                case 12:
                    return connector12;
                case 13:
                    return connector13;
                case 14:
                    return connector14;
                case 15:
                    return connector15;
                case 16:
                    return connector16;
                case 17:
                    return connector17;
                case 18:
                    return connector18;
                case 19:
                    return connector19;
                case 20:
                    return connector20;
                case 21:
                    return connector21;
                case 22:
                    return connector22;
                case 23:
                    return connector23;
                case 24:
                    return connector24;
                case 25:
                    return connector25;
                case 26:
                    return connector26;
                case 27:
                    return connector27;
                case 28:
                    return connector28;
            }
            //must be last connector. this is asserted at beginning of function
            return connector29;
        }
    };
}

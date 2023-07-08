
#pragma once
#include "MovableHandleBase.h"

namespace CurveAdjuster
{
    /* This handle can only move vertically. Primary intent
    is for start and end values in a curve adjuster.
    */
    class AdjustmentHandle1D : public MovableHandleBase
    {
    public:
        AdjustmentHandle1D(float diameter, pointType pos)
            : MovableHandleBase(diameter, pos)
        {}

        bool GetCanMoveHorizontally() override
        {
            return false;
        }
        bool GetCanMoveVertically() override
        {
            return true;
        }
        
        //these methods prevent from being selected in MultiSelect
        void SetSelectedForMultiSelect(bool) override
        {
        }
        bool GetIsSelectedInMultiSelect() override
        {
            return false;
        }
    };
}

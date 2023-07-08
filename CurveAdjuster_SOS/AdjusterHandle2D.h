#pragma once
#include "MovableHandleBase.h"

namespace CurveAdjuster
{
    class AdjustmentHandle2D : public MovableHandleBase
    {
    public:
        AdjustmentHandle2D(float diameter, pointType pos)
            : MovableHandleBase(diameter, pos)
        {}

        bool GetCanMoveHorizontally() override
        {
            return true;
        }
        bool GetCanMoveVertically() override
        {
            return true;
        }

    };
}
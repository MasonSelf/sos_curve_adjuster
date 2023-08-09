#pragma once

#if 0

BEGIN_JUCE_MODULE_DECLARATION

      ID:               sos_curve_adjuster
      vendor:           Mason Self
      version:          0.0.1
      name:             sos_curve_adjuster
      description:      CurveAdjusterSlidersSynthsOfSelf
      license:          GPL/Commercial
      dependencies:     juce_audio_utils, juce_gui_basics, juce_graphics, juce_audio_processors, sliders_sos, sos_maths

     END_JUCE_MODULE_DECLARATION

#endif

#include "CurveAdjuster_SOS/AdjusterHandle1D.h"
#include "CurveAdjuster_SOS/AdjusterHandle2D.h"
#include "CurveAdjuster_SOS/Connector.h"
#include "CurveAdjuster_SOS/CurveAdjusterComponent.h"
#include "CurveAdjuster_SOS/CurveAdjusterEditor.h"
#include "CurveAdjuster_SOS/CurveAdjusterPointTypes.h"
#include "CurveAdjuster_SOS/CurveAdjusterProcessor.h"
#include "CurveAdjuster_SOS/CurveAdjusterProcessorData.h"
#include "CurveAdjuster_SOS/DebugHelperFunctions.h"
#include "CurveAdjuster_SOS/IAdjusterHandle.h"
#include "CurveAdjuster_SOS/ICurveAdjusterEditor.h"
#include "CurveAdjuster_SOS/ICurveAdjusterProcessor.h"
#include "CurveAdjuster_SOS/MouseIgnoringComponent.h"
#include "CurveAdjuster_SOS/MovableHandleBase.h"
#include "CurveAdjuster_SOS/MultiSelectionManager.h"
#include "CurveAdjuster_SOS/SmoothedValueManager.h"
#include "CurveAdjuster_SOS/SOSUndoManager.h"
#include "CurveAdjuster_SOS/StationaryHandle.h"


#include "appcore/applicationObjects.h"
#include "widgets/laserWidget.h"
#include "widgets/splineWidget.h"
#include "widgets/volumeWidget.h"
#include "widgets/planeWidget.h"

#include <vtkVolume.h>
#include <vtkMatrix4x4.h>

#include <cereal/archives/json.hpp>

#include <iostream>
//==============================================================================
ApplicationObjects::ApplicationObjects() : 
	cutplane{std::make_unique<PlaneWidget>()}
{
	vtkNew<vtkVolume> vol;
	volume = std::make_unique<VolumeWidget>(vol);
}
//==============================================================================

//==============================================================================
ApplicationObjects::~ApplicationObjects() = default;
//==============================================================================
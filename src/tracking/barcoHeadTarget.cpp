#include "tracking/barcoHeadTarget.h"
#include "tracking/trackingUtils.h"
#include "barco/barcoSystem.h"

namespace tracking
{
//==============================================================================
BarcoHeadTarget::BarcoHeadTarget(std::shared_ptr<BarcoSystem> barcoSystem) : 
	m_BarcoSystem{barcoSystem}
{
	m_BarcoSystem->SetTrackerType(BarcoSystem::TrackerType::Internal);
}
//==============================================================================

//==============================================================================
BarcoHeadTarget::~BarcoHeadTarget() = default;
//==============================================================================

//==============================================================================
auto BarcoHeadTarget::getHeadPosition() const -> HeadPoseType
{
	return estimateHeadPoseFromEyePositions(m_BarcoSystem->GetEyePositions());
}
//==============================================================================
}  // end namespace tracking
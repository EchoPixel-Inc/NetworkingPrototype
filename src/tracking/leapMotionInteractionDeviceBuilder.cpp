#include "tracking/leapMotionInteractionDeviceBuilder.h"
#include "tracking/leapMotionInteractionDevice.h"

namespace tracking
{
//=============================================================================
LeapMotionInteractionDeviceBuilder::LeapMotionInteractionDeviceBuilder(
	const QHostAddress& hostAddress) :
	m_HostAddress{hostAddress}
{}
//=============================================================================

//=============================================================================
std::unique_ptr<InteractionDeviceInterface>
	LeapMotionInteractionDeviceBuilder::create() const
{
	return std::make_unique<LeapMotionInteractionDevice>(m_HostAddress);
}
//=============================================================================
}  // end namespace tracking
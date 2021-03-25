#include "tracking/vrInkInteractionDeviceBuilder.h"

#ifdef USE_VRINK
#	include "tracking/vrInkInteractionDevice.h"
#endif

namespace tracking
{
//=========================================================================
std::unique_ptr<InteractionDeviceInterface>
	VRInkInteractionDeviceBuilder::create() const
{
	std::unique_ptr<InteractionDeviceInterface> interactionDevice;

#ifdef USE_VRINK
	interactionDevice = std::make_unique<VRInkInteractionDevice>();
#endif

	return interactionDevice;
}
//=========================================================================

}  // end namespace tracking
#include "tracking/zSpaceInteractionDeviceBuilder.h"

#ifdef USE_ZSPACE
#	include "zspace/zSpaceContextManager.h"
#	include "tracking/zSpaceInteractionDevice.h"
#endif

namespace tracking
{
//==============================================================================
std::unique_ptr<InteractionDeviceInterface>
	zSpaceInteractionDeviceBuilder::create() const
{
	std::unique_ptr<InteractionDeviceInterface> interactionDevice;

#ifdef USE_ZSPACE
	auto contextManager = zspace::ContextManager::defaultContextManager();
	if (!contextManager) {
		throw std::runtime_error("No zspace context available");
	}

	interactionDevice =
		std::make_unique<zSpaceInteractionDevice>(contextManager);
#endif
	return interactionDevice;
}
//==============================================================================
}  // end namespace tracking
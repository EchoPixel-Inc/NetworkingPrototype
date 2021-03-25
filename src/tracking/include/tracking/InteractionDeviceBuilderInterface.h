#ifndef interactionDeviceBuilderInterface_h
#define interactionDeviceBuilderInterface_h

#include "tracking/interactionDeviceInterface.h"

namespace tracking {
	class InteractionDeviceBuilderInterface
	{
	public:
		virtual std::unique_ptr<InteractionDeviceInterface> create() const = 0;
	};
} // end namespace tracking
#endif
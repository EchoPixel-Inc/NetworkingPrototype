#ifndef interactionDeviceInterface_h
#define interactionDeviceInterface_h

#include "tracking/trackingTypes.h"

#include <functional>

namespace tracking
{
	class InteractionDeviceInterface
	{
	public:
		using MoveCallbackType = std::function<void(const DevicePoseType&)>;
		using ButtonPressCallbackType = std::function<void()>;
		using ButtonReleaseCallbackType = std::function<void()>;

		InteractionDeviceInterface();
		virtual ~InteractionDeviceInterface() = 0;

		virtual DevicePoseType getPose() const = 0;

		virtual void setDeviceMovedCallback(MoveCallbackType) = 0;
		virtual void setButtonPressCallback(ButtonPressCallbackType) = 0;
		virtual void setButtonReleaseCallback(ButtonReleaseCallbackType) = 0;
	};
} // end namespace tracking
#endif
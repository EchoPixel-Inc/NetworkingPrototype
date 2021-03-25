#ifndef zSpaceInteractionDevice_h
#define zSpaceInteractionDevice_h

#include "tracking/interactionDeviceInterface.h"

#include "zSpace.h"

#include <functional>
#include <shared_mutex>

namespace zspace {
	class ContextManager;
};

namespace tracking
{
	class zSpaceInteractionDevice : public InteractionDeviceInterface
	{
	public:
		explicit zSpaceInteractionDevice(std::shared_ptr<::zspace::ContextManager>);
		virtual ~zSpaceInteractionDevice();

		virtual DevicePoseType getPose() const override;

		virtual void setDeviceMovedCallback(MoveCallbackType) override;
		virtual void setButtonPressCallback(ButtonPressCallbackType) override;
		virtual void setButtonReleaseCallback(ButtonReleaseCallbackType) override;

	protected:
		void onStylusMoved();
		void onButtonPressed();
		void onButtonReleased();

		static void onStylusMovedImpl(ZCHandle, const ZCTrackerEventData*,
			const void*);

		static void onButtonPressedImpl(ZCHandle, const ZCTrackerEventData*,
			const void*);

		static void onButtonReleasedImpl(ZCHandle, const ZCTrackerEventData*,
			const void*);

	private:
		ZCHandle m_StylusHandle;
		mutable std::shared_mutex m_Mutex;
		DevicePoseType m_CurrentPose;
		std::shared_ptr<::zspace::ContextManager> m_ContextManager;
		std::function<void(const DevicePoseType&)> m_StylusMovedCallback;
		std::function<void()> m_StylusButtonPressedCallback;
		std::function<void()> m_StylusButtonReleasedCallback;
	};
}  // end namespace tracking
#endif
#ifndef vrInkInteractionDevice_h
#define vrInkInteractionDevice_h

#include "tracking/interactionDeviceInterface.h"

#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace tracking
{
class VRInkInteractionDevice : public InteractionDeviceInterface
{
public:
	explicit VRInkInteractionDevice();
	virtual ~VRInkInteractionDevice();

	virtual DevicePoseType getPose() const override;

	virtual void setDeviceMovedCallback(MoveCallbackType) override;
	virtual void setButtonPressCallback(ButtonPressCallbackType) override;
	virtual void setButtonReleaseCallback(ButtonReleaseCallbackType) override;

private:
	struct DeviceInfo
	{
		DevicePoseType pose = DevicePoseType::Identity();
		bool connected = false;
	};

	void runUpdateLoop();

	DeviceInfo m_DeviceInfo;
	std::thread m_UpdateThread;
	std::atomic<bool> m_ThreadAbortFlag;
	mutable std::mutex m_CallbackMutex;
	mutable std::shared_mutex m_DeviceMutex;
	MoveCallbackType m_MoveCallback;
	ButtonPressCallbackType m_ButtonPressCallback;
	ButtonReleaseCallbackType m_ButtonReleaseCallback;
};
}  // end namespace tracking

#endif
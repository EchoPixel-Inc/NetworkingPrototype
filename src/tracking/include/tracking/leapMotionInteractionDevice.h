#ifndef leapMotionInteractionDevice_h
#define leapMotionInteractionDevice_h

#include "tracking/interactionDeviceInterface.h"
#include "tracking/trackingTypes.h"

#include <QHostAddress>

#include <functional>
#include <memory>

class QObject;
class QThread;

namespace tracking
{
class LeapMotionClient;

class LeapMotionInteractionDevice : public InteractionDeviceInterface
{
public:
	explicit LeapMotionInteractionDevice(const QHostAddress&);
	virtual ~LeapMotionInteractionDevice();

	virtual DevicePoseType getPose() const override;

	virtual void setDeviceMovedCallback(MoveCallbackType) override;
	virtual void setButtonPressCallback(ButtonPressCallbackType) override;
	virtual void setButtonReleaseCallback(ButtonReleaseCallbackType) override;

private:
	std::unique_ptr<LeapMotionClient> m_LeapClient;
	std::unique_ptr<QThread> m_TrackingThread;
	std::unique_ptr<QObject> m_ContextObj;
};
}  // namespace tracking
#endif
#ifndef customQEvents_h
#define customQEvents_h

#include "tracking/trackingTypes.h"

#include <QEvent>

#include <type_traits>

enum CustomQEvents : std::underlying_type_t<QEvent::Type> {
	DEVICE_MOVE = QEvent::User + 1,
	DEVICE_BUTTONPRESS,
	DEVICE_BUTTONRELEASE
};

class DeviceMoveEvent : public QEvent
{
public:
	DeviceMoveEvent(const tracking::DevicePoseType& devicePose) :
		QEvent{static_cast<QEvent::Type>(CustomQEvents::DEVICE_MOVE)},
		devicePose{devicePose}
	{}

	tracking::DevicePoseType devicePose;
};

class DeviceButtonPressEvent : public QEvent
{
public:
	DeviceButtonPressEvent() :
		QEvent{static_cast<QEvent::Type>(CustomQEvents::DEVICE_BUTTONPRESS)}
	{}
};

class DeviceButtonReleaseEvent : public QEvent
{
public:
	DeviceButtonReleaseEvent() :
		QEvent{static_cast<QEvent::Type>(CustomQEvents::DEVICE_BUTTONRELEASE)}
	{}
};

#endif
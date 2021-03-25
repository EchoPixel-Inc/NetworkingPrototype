#ifndef leapMotionInteractionDeviceBuilder_h
#define leapMotionInteractionDeviceBuilder_h

#include "tracking/InteractionDeviceBuilderInterface.h"

#include <QHostAddress>

namespace tracking
{
class LeapMotionInteractionDeviceBuilder :
	public InteractionDeviceBuilderInterface
{
public:
	LeapMotionInteractionDeviceBuilder(const QHostAddress&);

	virtual std::unique_ptr<InteractionDeviceInterface> create() const;

private:
	QHostAddress m_HostAddress;
};
}  // end namespace tracking
#endif
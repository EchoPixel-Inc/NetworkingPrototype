#ifndef vrInkInteractionDeviceBuilder_h
#define vrInkInteractionDeviceBuilder_h

#include "tracking/InteractionDeviceBuilderInterface.h"

namespace tracking
{
class VRInkInteractionDeviceBuilder : public InteractionDeviceBuilderInterface
{
public:
	virtual std::unique_ptr<InteractionDeviceInterface> create() const override;
};
}  // end namespace tracking

#endif
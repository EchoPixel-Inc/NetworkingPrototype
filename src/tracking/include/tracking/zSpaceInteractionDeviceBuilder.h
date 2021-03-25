#ifndef zSpaceInteractionDeviceBuilder_h
#define zSpaceInteractionDeviceBuilder_h

#include "tracking/InteractionDeviceBuilderInterface.h"

namespace tracking
{
class zSpaceInteractionDeviceBuilder : public InteractionDeviceBuilderInterface
{
public:
	virtual std::unique_ptr<InteractionDeviceInterface> create() const override;
};
}  // namespace tracking

#endif
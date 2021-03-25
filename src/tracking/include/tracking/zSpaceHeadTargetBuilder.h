#ifndef zSpaceHeadTargetBuilder_h
#define zSpaceHeadTargetBuilder_h

#include "tracking/headTargetBuilderInterface.h"

namespace tracking
{
class zSpaceHeadTargetBuilder : public HeadTargetBuilderInterface
{
public:
	virtual std::unique_ptr<HeadTargetInterface> create() const override;
};
}  // end namespace tracking

#endif
#ifndef barcoHeadTargetBuilder_h
#define barcoHeadTargetBuidler_h

#include "tracking/headTargetBuilderInterface.h"

namespace tracking
{
class BarcoHeadTargetBuilder : public HeadTargetBuilderInterface
{
public:
	virtual std::unique_ptr<HeadTargetInterface> create() const override;
};
}  // end namespace tracking

#endif

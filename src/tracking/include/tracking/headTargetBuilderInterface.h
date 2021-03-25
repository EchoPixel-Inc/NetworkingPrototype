#ifndef headTargetBuilderInterface_h
#define headTargetBuilderInterface_h

#include "tracking/headTargetInterface.h"

namespace tracking
{
class HeadTargetBuilderInterface
{
public:
	virtual std::unique_ptr<HeadTargetInterface> create() const = 0;
};
}  // end namespace tracking
#endif
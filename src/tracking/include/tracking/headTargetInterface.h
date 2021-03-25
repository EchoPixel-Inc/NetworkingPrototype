#ifndef headTargetInterface_h
#define headTargetInterface_h

#include "tracking/trackingTypes.h"

namespace tracking
{
class HeadTargetInterface
{
public:
	HeadTargetInterface();
	virtual ~HeadTargetInterface();

	virtual HeadPoseType getHeadPosition() const = 0;
};
}  // namespace tracking
#endif
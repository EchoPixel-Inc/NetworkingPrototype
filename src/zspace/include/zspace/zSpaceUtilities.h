#ifndef zSpaceUtilities_h
#define zSpaceUtilities_h

#include "common/coreTypes.h"

#include "zSpace.h"

#include <string>
#include <memory>

namespace zspace
{
	std::string getErrorString(ZCError error);
	common::TransformType convertPose(const ZCTrackerPose& trackerPose);
}

#endif
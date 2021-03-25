#ifndef displayInterface_h
#define displayInterface_h

#include "common/coreTypes.h"
#include "display/displayInfo.h"

#include <array>
#include <string>
#include <memory>

class DisplayInterface
{
public:
	using PointType = std::array<double, 2>;
	using PoseType = common::TransformType;
	using EyePositionsType = common::EyePositions;

	virtual const DisplayInfo& getInfo() const = 0;
	virtual PoseType getPhysicalPose() const = 0;
	virtual void syncStereoBuffers() = 0;
	virtual void sendEyePositions(const EyePositionsType&) = 0;

	static std::shared_ptr<DisplayInterface> getDefaultImplementation();
};
#endif
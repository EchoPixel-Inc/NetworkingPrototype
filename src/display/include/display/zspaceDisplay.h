#ifndef zSpaceDisplay_h
#define zSpaceDisplay_h

#include "display/displayInterface.h"

#include "zSpace.h"

#include <memory>

namespace zspace {
	class ContextManager;
}

class zSpaceDisplay : public DisplayInterface
{
public:
	explicit zSpaceDisplay(const DisplayInfo& displayInfo,
		std::shared_ptr<zspace::ContextManager> contextManager);

	virtual ~zSpaceDisplay();

	virtual const DisplayInfo& getInfo() const override;
	virtual PoseType getPhysicalPose() const override;
	virtual void syncStereoBuffers() override;
	virtual void sendEyePositions(const EyePositionsType&) override;

private:
	std::shared_ptr<zspace::ContextManager> m_ContextManager;
	ZCHandle m_DisplayHandle;
	ZCHandle m_StereoBufferHandle;
	const DisplayInfo m_Info;
};

#endif
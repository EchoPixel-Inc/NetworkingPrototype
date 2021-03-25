#ifndef genericDisplay_h
#define genericDisplay_h

#include "display/displayInterface.h"

class GenericDisplay : public DisplayInterface
{
public:
	GenericDisplay(const DisplayInfo&);

	virtual const DisplayInfo& getInfo() const override;
	virtual PoseType getPhysicalPose() const override;
	virtual void syncStereoBuffers() override;
	virtual void sendEyePositions(const EyePositionsType&) override;
private:
	const DisplayInfo m_Info;
};

#endif
#ifndef barcoDisplay_h
#define barcoDisplay_h

#include "display/displayInterface.h"

#include <memory>

class BarcoSystem;

class BarcoDisplay : public DisplayInterface
{
public:
	explicit BarcoDisplay(const DisplayInfo& displayInfo,
		std::shared_ptr<BarcoSystem> barcoSystem);

	virtual ~BarcoDisplay();

	virtual const DisplayInfo& getInfo() const override;
	virtual PoseType getPhysicalPose() const override;
	virtual void syncStereoBuffers() override;
	virtual void sendEyePositions(const EyePositionsType&) override;

private:
	std::shared_ptr<BarcoSystem> m_BarcoSystem;
	const DisplayInfo m_Info;
};

#endif
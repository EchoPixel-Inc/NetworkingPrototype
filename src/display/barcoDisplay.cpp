#include "display/barcoDisplay.h"
#include "barco/barcoSystem.h"

//=============================================================================
BarcoDisplay::BarcoDisplay(const DisplayInfo& displayInfo,
	std::shared_ptr<BarcoSystem> barcoSystem) : 
	m_Info{displayInfo},
	m_BarcoSystem{barcoSystem}
{}
//=============================================================================

//=============================================================================
BarcoDisplay::~BarcoDisplay() = default;
//=============================================================================

//=============================================================================
auto BarcoDisplay::getInfo() const -> const DisplayInfo&
{
	return m_Info;
}
//=============================================================================

//=============================================================================
auto BarcoDisplay::getPhysicalPose() const -> PoseType
{
	return PoseType::Identity();
}
//=============================================================================

//=============================================================================
void BarcoDisplay::syncStereoBuffers() {}
//=============================================================================

//=============================================================================
void BarcoDisplay::sendEyePositions(const EyePositionsType& eyePositions)
{
	m_BarcoSystem->SetEyePositions(eyePositions);
}
//=============================================================================
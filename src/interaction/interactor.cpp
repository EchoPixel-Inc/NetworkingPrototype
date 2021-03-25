#include "interaction/interactor.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(Interactor);
//=============================================================================

//=============================================================================
Interactor::Interactor() : 
	DevicePose{ DevicePoseType::Identity() },
	LastDevicePose{ DevicePoseType::Identity() },
	m_InteractionRayLength{150.0}
{
	this->SetInteractorStyle(nullptr);
}
//=============================================================================

//=============================================================================
Interactor::~Interactor() = default;
//=============================================================================

//=============================================================================
void Interactor::Enable()
{
	if (this->Enabled) {
		return;
	}

	this->Enabled = 1;
	this->Modified();
}
//=============================================================================

//=============================================================================
void Interactor::Disable()
{
	if (!this->Enabled) {
		return;
	}

	this->Enabled = 0;
	this->Modified();
}
//=============================================================================

//=============================================================================
void Interactor::SetDevicePose(const DevicePoseType& devicePose)
{
	this->LastDevicePose = this->DevicePose;
	this->DevicePose = devicePose;
}
//=============================================================================

//=============================================================================
auto Interactor::GetDevicePose() const -> const DevicePoseType&
{
	return this->DevicePose;
}
//=============================================================================

//=============================================================================
auto Interactor::GetLastDevicePose() const -> const DevicePoseType&
{
	return this->LastDevicePose;
}
//=============================================================================

//=============================================================================
void Interactor::SetInteractionRayLength(double rayLength) 
{
	m_InteractionRayLength = rayLength;
}
//=============================================================================

//=============================================================================
double Interactor::GetInteractionRayLength() const
{
	return m_InteractionRayLength;
}
//=============================================================================

//=============================================================================
void Interactor::PrintSelf(std::ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}
//=============================================================================
#ifndef interactor_h
#define interactor_h

#include "common/coreTypes.h"

#include <vtkRenderWindowInteractor.h>

#include <iostream>

class Interactor : public vtkRenderWindowInteractor
{
public:
	using DevicePoseType = common::TransformType;
	using Point3dType = common::Point3dType;

	static Interactor* New();

	vtkTypeMacro(Interactor, vtkRenderWindowInteractor);
	void PrintSelf(std::ostream&, vtkIndent indent) override;

	void Enable() override;
	void Disable() override;

	void SetInteractionRayLength(double);
	double GetInteractionRayLength() const;

	void SetDevicePose(const DevicePoseType&);
	const DevicePoseType& GetDevicePose() const;
	const DevicePoseType& GetLastDevicePose() const;

protected:
	Interactor();
	~Interactor() override;

private:
	Interactor(const Interactor&) = delete;
	void operator=(const Interactor&) = delete;

	DevicePoseType DevicePose;
	DevicePoseType LastDevicePose;
	double m_InteractionRayLength;
};

#endif
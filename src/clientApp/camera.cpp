/*=============================================================================*/
/// \file Camera.cpp
/// \author Jeffrey Kasten (jeffrey@echopixeltech.com)
///	\date 2020/05/07
///	\version
///	\copyright (C) EchoPixel, Inc. 2020. All rights reserved.
/*=============================================================================*/
#include "clientApp/camera.h"

#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include "vtkOpenGLError.h"
#include "vtkOpenGL.h"
#include <vtkTransform.h>
#include <vtkRenderer.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkPerspectiveTransform.h>
#include <vtkOpenGLState.h>

#include <cmath>
#include <array>
#include <numeric>

//==============================================================================
Camera::Camera() : UseExplicitEyePositions{false}
{
	this->LeftEyePosition[0] = this->Position[0] - this->EyeSeparation;
	this->LeftEyePosition[1] = this->Position[1];
	this->LeftEyePosition[2] = this->Position[2];

	this->RightEyePosition[0] = this->Position[0] + this->EyeSeparation;
	this->RightEyePosition[1] = this->Position[1];
	this->RightEyePosition[2] = this->Position[2];

	this->SetClippingRange(0.001, 1.0);
	this->SetFocalPoint(0.0, 0.0, 0.0);
	this->SetViewUp(0.0, 1.0, 0.0);
}
//==============================================================================

//==============================================================================
Camera::~Camera()
{
}
//==============================================================================

//==============================================================================
Camera* Camera::New()
{
	return new Camera();
}
//==============================================================================


#include <chrono>
using namespace std::chrono_literals;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::duration_cast;
high_resolution_clock::time_point startTime1;
high_resolution_clock::time_point endTime1;

//==============================================================================
void Camera::ResetClippingRange(const double bounds[6])
{
	const auto& xMin = bounds[0];
	const auto& xMax = bounds[1];
	const auto& yMin = bounds[2];
	const auto& yMax = bounds[3];
	const auto& zMin = bounds[4];
	const auto& zMax = bounds[5];

	double position[3];
	double nrml[3];

	if (this->GetUseOffAxisProjection()) {
		this->GetEyePlaneNormal(nrml);

		// Need to use the eye position closest to the screen to determine
		// the appropriate clipping range.  This should be the eye position with
		// the smallest projection onto the view plane normal
		auto leftEyeDistance =
			std::inner_product(std::begin(this->LeftEyePosition),
				std::end(this->LeftEyePosition), std::begin(nrml), 0.0);

		auto rightEyeDistance =
			std::inner_product(std::begin(this->RightEyePosition),
				std::end(this->RightEyePosition), std::begin(nrml), 0.0);

		if (leftEyeDistance < rightEyeDistance) {
			std::copy(std::begin(this->LeftEyePosition),
				std::end(this->LeftEyePosition), std::begin(position));
		}
		else {
			std::copy(std::begin(this->RightEyePosition),
				std::end(this->RightEyePosition), std::begin(position));
		}
	}
	else {
		this->GetPosition(position);
		this->GetViewPlaneNormal(nrml);
	}

	using ArrayType = std::array<double, 4>;
	ArrayType projVec{-1.0 * nrml[0], -1.0 * nrml[1],
		-1.0 * nrml[2],
		nrml[0] * position[0] + nrml[1] * position[1] +
			nrml[2] * position[2]};

	using ArrayContainerType = std::array<ArrayType, 8>;
	ArrayContainerType boundaryPts{
		{{xMin, yMin, zMin, 1.0}, {xMax, yMin, zMin, 1.0},
			{xMin, yMin, zMax, 1.0}, {xMax, yMin, zMax, 1.0},
			{xMin, yMax, zMin, 1.0}, {xMax, yMax, zMin, 1.0},
			{xMin, yMax, zMax, 1.0}, {xMax, yMax, zMax, 1.0}}};

	double range[2] = {std::numeric_limits<ArrayType::value_type>::max(),
		std::numeric_limits<ArrayType::value_type>::min()};

	for (const auto& pt : boundaryPts) {
		auto val =
			std::inner_product(projVec.begin(), projVec.end(), pt.begin(), 0.0);

		range[0] = (std::min)(range[0], val);
		range[1] = (std::max)(range[1], val);
	}

	// Do not let far - near be less than 0.1 of the window height.
	// This is for cases such as 2D images which may have zero range
	double minGap{0.0};
	if (this->ParallelProjection) {
		minGap = 0.1 * this->ParallelScale;
	}
	else {
		double angle = vtkMath::RadiansFromDegrees(this->ViewAngle);
		minGap = 0.2 * std::tan(0.5 * angle) * range[1];
	}

	if (range[1] - range[0] < minGap) {
		minGap = minGap - range[1] + range[0];
		range[1] += minGap / 2.0;
		range[0] -= minGap / 2.0;
	}

	// Don't let the range behind the camera throw off calculation
	range[0] = std::max(0.0, range[0]);

	// Expand the range by 5% in either direction to give us a buffer
	range[0] -= 0.05 * (range[1] - range[0]);
	range[1] += 0.05 * (range[1] - range[0]);

	// Make sure near plane is at least soem fraction of the far plane, which
	// prevents the near plane from being behyind the camera or too close in
	// front.  How close is too close depends on the resolution of the depth
	// buffer
	double nearPlaneClipTolerance{0.01};

	// Make sure the front clipping plane is not too far from the far clipping
	// range to make sure teh zbuffer resolution is used effectively
	if (range[0] < nearPlaneClipTolerance * range[1]) {
		range[0] = nearPlaneClipTolerance * range[1];
	}

	SetClippingRange(range[0], range[1]);
}
//==============================================================================

//==============================================================================
void Camera::ResetClippingRange(double xmin, double xmax,
	double ymin, double ymax, double zmin, double zmax)
{
	double bounds[6] = {xmin, xmax, ymin, ymax, zmin, zmax};
	this->ResetClippingRange(bounds);
}
//==============================================================================

//==============================================================================
void Camera::SetUseExplicitEyePositions(
	bool useExplicitEyePositions)
{
	this->UseExplicitEyePositions = useExplicitEyePositions;
}
//==============================================================================

//==============================================================================
bool Camera::GetUseExplicitEyePositions() const
{
	return this->UseExplicitEyePositions;
}
//==============================================================================

//==============================================================================
void Camera::SetLeftEyePosition(double x, double y, double z)
{
	this->LeftEyePosition[0] = x;
	this->LeftEyePosition[1] = y;
	this->LeftEyePosition[2] = z;

	this->Modified();
}
//==============================================================================

//==============================================================================
void Camera::SetLeftEyePosition(const double leftEyePosition[3])
{
	this->SetLeftEyePosition(
		leftEyePosition[0], leftEyePosition[1], leftEyePosition[2]);
}
//==============================================================================

//==============================================================================
double* Camera::GetLeftEyePosition()
{
	return this->LeftEyePosition;
}
//==============================================================================

//==============================================================================
void Camera::GetLeftEyePosition(
	double& x, double& y, double& z) const
{
	x = this->LeftEyePosition[0];
	y = this->LeftEyePosition[1];
	z = this->LeftEyePosition[2];
}
//==============================================================================

//==============================================================================
void Camera::GetLeftEyePosition(double leftEyePosition[3]) const
{
	this->GetLeftEyePosition(
		leftEyePosition[0], leftEyePosition[1], leftEyePosition[2]);
}
//==============================================================================

//==============================================================================
void Camera::SetRightEyePosition(double x, double y, double z)
{
	this->RightEyePosition[0] = x;
	this->RightEyePosition[1] = y;
	this->RightEyePosition[2] = z;

	this->Modified();
}
//==============================================================================

//==============================================================================
void Camera::SetRightEyePosition(const double rightEyePosition[3])
{
	this->SetRightEyePosition(
		rightEyePosition[0], rightEyePosition[1], rightEyePosition[2]);
}
//==============================================================================

//==============================================================================
double* Camera::GetRightEyePosition()
{
	return this->RightEyePosition;
}
//==============================================================================

//==============================================================================
void Camera::GetRightEyePosition(
	double& x, double& y, double& z) const
{
	x = this->RightEyePosition[0];
	y = this->RightEyePosition[1];
	z = this->RightEyePosition[2];
}
//==============================================================================

//==============================================================================
void Camera::GetRightEyePosition(double rightEyePosition[3]) const
{
	this->GetRightEyePosition(
		rightEyePosition[0], rightEyePosition[1], rightEyePosition[2]);
}
//==============================================================================

//==============================================================================
void Camera::ComputeEyePositions()
{
	double leftEyePosition[4] = {-0.5 * this->EyeSeparation, 0.0, 0.0, 1.0};
	double rightEyePosition[4] = {0.5 * this->EyeSeparation, 0.0, 0.0, 1.0};

	this->EyeTransformMatrix->MultiplyPoint(leftEyePosition, leftEyePosition);
	this->LeftEyePosition[0] = leftEyePosition[0];
	this->LeftEyePosition[1] = leftEyePosition[1];
	this->LeftEyePosition[2] = leftEyePosition[2];

	this->EyeTransformMatrix->MultiplyPoint(rightEyePosition, rightEyePosition);
	this->RightEyePosition[0] = rightEyePosition[0];
	this->RightEyePosition[1] = rightEyePosition[1];
	this->RightEyePosition[2] = rightEyePosition[2];

	this->EyePositionsMTime.Modified();
}
//==============================================================================

//==============================================================================
void Camera::SetScreenBottomLeft(double x, double y, double z)
{
	this->ScreenBottomLeft[0] = x;
	this->ScreenBottomLeft[1] = y;
	this->ScreenBottomLeft[2] = z;

	this->ComputeWorldToDisplayMatrix();
}
//==============================================================================

//==============================================================================
void Camera::SetScreenBottomRight(double x, double y, double z)
{
	this->ScreenBottomRight[0] = x;
	this->ScreenBottomRight[1] = y;
	this->ScreenBottomRight[2] = z;

	this->ComputeWorldToDisplayMatrix();
}
//==============================================================================

//==============================================================================
void Camera::SetScreenTopRight(double x, double y, double z)
{
	this->ScreenTopRight[0] = x;
	this->ScreenTopRight[1] = y;
	this->ScreenTopRight[2] = z;

	this->ComputeWorldToDisplayMatrix();
}
//==============================================================================

//==============================================================================
vtkMatrix4x4* Camera::GetModelViewTransformMatrix()
{
	return this->GetModelViewTransformObject()->GetMatrix();
}
//==============================================================================

//==============================================================================
vtkTransform* Camera::GetModelViewTransformObject()
{
	if (this->UseOffAxisProjection) {
		if (this->ViewTransform->GetMTime() < this->GetMTime()) {
			this->ComputeViewTransform();
		}

		if (this->ModelViewTransform->GetMTime() <
			this->ModelTransformMatrix->GetMTime() ||
			this->ModelViewTransform->GetMTime() <
			this->ViewTransform->GetMTime()) {
			vtkMatrix4x4::Multiply4x4(this->ViewTransform->GetMatrix(),
				this->ModelTransformMatrix, this->ModelViewTransform->GetMatrix());

			this->ModelViewTransform->Identity();
			this->ModelViewTransform->Concatenate(this->ViewTransform);
			this->ModelViewTransform->Concatenate(this->ModelTransformMatrix);
		}

		return this->ModelViewTransform;
	}

	return Superclass::GetModelViewTransformObject();
}
//==============================================================================

//==============================================================================
void Camera::ComputeViewTransform()
{
	if (this->UseOffAxisProjection) {
		this->ViewTransform->Identity();

		if (this->UserViewTransform) {
			this->ViewTransform->Concatenate(
				this->UserViewTransform->GetMatrix());
		}

		if ((!this->UseExplicitEyePositions) &&
			(EyePositionsMTime.GetMTime() < this->GetMTime())) {
			this->ComputeEyePositions();
		}

		double eyePosition[4] = {0.0, 0.0, 0.0, 1.0};
		if (this->LeftEye) {
			eyePosition[0] = this->LeftEyePosition[0];
			eyePosition[1] = this->LeftEyePosition[1];
			eyePosition[2] = this->LeftEyePosition[2];
		}
		else {
			eyePosition[0] = this->RightEyePosition[0];
			eyePosition[1] = this->RightEyePosition[1];
			eyePosition[2] = this->RightEyePosition[2];
		}

		vtkNew<vtkMatrix4x4> eyeTranslation;
		eyeTranslation->Identity();

		eyeTranslation->SetElement(0, 3, -1.0 * eyePosition[0]);
		eyeTranslation->SetElement(1, 3, -1.0 * eyePosition[1]);
		eyeTranslation->SetElement(2, 3, -1.0 * eyePosition[2]);

		this->ViewTransform->Concatenate(this->WorldToScreenMatrix);
		this->ViewTransform->Concatenate(eyeTranslation);

		// Now we need to retroactively determine the position and focal point
		// of the camera
		this->Position[0] = eyePosition[0];
		this->Position[1] = eyePosition[1];
		this->Position[2] = eyePosition[2];

		double v_a[3] = {this->ScreenBottomLeft[0] - eyePosition[0],
			this->ScreenBottomLeft[1] - eyePosition[1],
			this->ScreenBottomLeft[2] - eyePosition[2]};

		// Compute distance from eye to display-space origin
		// First extract the screen normal from the world to Display transform
		double v_n[3] = {this->WorldToScreenMatrix->GetElement(2, 0),
			this->WorldToScreenMatrix->GetElement(2, 1),
			this->WorldToScreenMatrix->GetElement(2, 2)};

		double d = std::abs(vtkMath::Dot(v_n, v_a));

		this->FocalPoint[0] = -1.0 * d * v_n[0] + this->Position[0];
		this->FocalPoint[1] = -1.0 * d * v_n[1] + this->Position[1];
		this->FocalPoint[2] = -1.0 * d * v_n[2] + this->Position[2];

		ComputeDistance();
		ComputeCameraLightTransform();

		this->Modified();
	}
	else {
		Superclass::ComputeViewTransform();
	}
}
//==============================================================================

//==============================================================================
void Camera::ComputeWorldToDisplayMatrix()
{
	double xAxis[3];
	double yAxis[3];
	double zAxis[3];

	for (unsigned int i = 0; i < 3; i++) {
		xAxis[i] = this->ScreenBottomRight[i] - this->ScreenBottomLeft[i];
		yAxis[i] = this->ScreenTopRight[i] - this->ScreenBottomRight[i];
	}

	vtkMath::Normalize(xAxis);
	vtkMath::Normalize(yAxis);
	vtkMath::Cross(xAxis, yAxis, zAxis);
	vtkMath::Normalize(zAxis);

	this->WorldToScreenMatrix->Identity();

	this->WorldToScreenMatrix->SetElement(0, 0, xAxis[0]);
	this->WorldToScreenMatrix->SetElement(1, 0, xAxis[1]);
	this->WorldToScreenMatrix->SetElement(2, 0, xAxis[2]);

	this->WorldToScreenMatrix->SetElement(0, 1, yAxis[0]);
	this->WorldToScreenMatrix->SetElement(1, 1, yAxis[1]);
	this->WorldToScreenMatrix->SetElement(2, 1, yAxis[2]);

	this->WorldToScreenMatrix->SetElement(0, 2, zAxis[0]);
	this->WorldToScreenMatrix->SetElement(1, 2, zAxis[1]);
	this->WorldToScreenMatrix->SetElement(2, 2, zAxis[2]);

	this->WorldToScreenMatrix->Invert();
	this->WorldToScreenMatrixMTime.Modified();
}
//==============================================================================

//==============================================================================
void Camera::ComputeProjectionTransform(
	double aspect, double nearPlaneZ, double farPlaneZ)
{
	if (this->UseOffAxisProjection) {
		this->ProjectionTransform->Identity();
		this->ProjectionTransform->AdjustZBuffer(-1, 1, nearPlaneZ, farPlaneZ);

		if ((!this->UseExplicitEyePositions) &&
			(EyePositionsMTime.GetMTime() < this->GetMTime())) {
			this->ComputeEyePositions();
		}

		double eyePosition[4] = {0.0, 0.0, 0.0, 1.0};

		if (this->LeftEye) {
			eyePosition[0] = this->LeftEyePosition[0];
			eyePosition[1] = this->LeftEyePosition[1];
			eyePosition[2] = this->LeftEyePosition[2];
		}
		else {
			eyePosition[0] = this->RightEyePosition[0];
			eyePosition[1] = this->RightEyePosition[1];
			eyePosition[2] = this->RightEyePosition[2];
		}

		// Calculate the vectors from the eye position to the screen corner
		double v_a[3];
		double v_b[3];
		double v_c[3];

		// Compute the top-left corner of the screen
		double u[3];
		double screenTopLeft[3];
		for (unsigned int i = 0; i < 3; i++) {
			u[i] = this->ScreenTopRight[i] - this->ScreenBottomRight[i];
			screenTopLeft[i] = this->ScreenBottomLeft[i] + u[i];
		}

		for (unsigned int i = 0; i < 3; i++) {
			v_a[i] = this->ScreenBottomLeft[i] - eyePosition[i];
			v_b[i] = this->ScreenBottomRight[i] - eyePosition[i];
			v_c[i] = screenTopLeft[i] - eyePosition[i];
		}

		// z-normal
		double v_n[3] = {this->WorldToScreenMatrix->GetElement(2, 0),
			this->WorldToScreenMatrix->GetElement(2, 1),
			this->WorldToScreenMatrix->GetElement(2, 2)};

		// x-normal
		double v_r[3] = {this->WorldToScreenMatrix->GetElement(0, 0),
			this->WorldToScreenMatrix->GetElement(0, 1),
			this->WorldToScreenMatrix->GetElement(0, 2)};

		// y-normal
		double v_u[3] = {this->WorldToScreenMatrix->GetElement(1, 0),
			this->WorldToScreenMatrix->GetElement(1, 1),
			this->WorldToScreenMatrix->GetElement(1, 2)};

		// Compute distance from eye to display-space origin
		double d = -1.0 * vtkMath::Dot(v_n, v_a);

		// Compute additional parameters needed to calculate the frustrum
		double n = this->ClippingRange[0];
		double f = this->ClippingRange[1];
		double l = (n / d) * vtkMath::Dot(v_r, v_a);
		double r = (n / d) * vtkMath::Dot(v_r, v_b);
		double b = (n / d) * vtkMath::Dot(v_u, v_a);
		double t = (n / d) * vtkMath::Dot(v_u, v_c);

		vtkNew<vtkMatrix4x4> projMatrix;
		projMatrix->Identity();

		projMatrix->SetElement(0, 0, (2 * n) / (r - l));
		projMatrix->SetElement(0, 2, (r + l) / (r - l));
		projMatrix->SetElement(1, 1, (2 * n) / (t - b));
		projMatrix->SetElement(1, 2, (t + b) / (t - b));
		projMatrix->SetElement(2, 2, -1.0 * (f + n) / (f - n));
		projMatrix->SetElement(2, 3, -2.0 * (f * n) / (f - n));
		projMatrix->SetElement(3, 2, -1.0);
		projMatrix->SetElement(3, 3, 0.0);

		this->ProjectionTransform->SetMatrix(projMatrix);
	}
	else {
		Superclass::ComputeProjectionTransform(aspect, nearPlaneZ, farPlaneZ);
	}
}
//==============================================================================

//==============================================================================
void Camera::PrintSelf(std::ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Use Explicit Eye Positions: " << std::boolalpha
	   << this->UseExplicitEyePositions << "\n";

	os << indent << "Left Eye Position: (" << this->LeftEyePosition[0] << ", "
	   << this->LeftEyePosition[1] << ", " << this->LeftEyePosition[2] << ")"
	   << "\n";

	os << indent << "Right Eye Position: (" << this->RightEyePosition[0] << ", "
	   << this->RightEyePosition[1] << ", " << this->RightEyePosition[2] << ")"
	   << "\n";
}
//==============================================================================
/*=============================================================================*/
/// \file Camera.h
/// \author Jeffrey Kasten (jeffrey@echopixeltech.com)
///	\date 2020/05/07
///	\version 
///	\copyright (C) EchoPixel, Inc. 2020. All rights reserved.
/*=============================================================================*/
#ifndef Camera_h
#define Camera_h

#include <vtkOpenGLCamera.h>

class vtkRenderer;

/// \class Camera
/// \brief This class is a subclass of vtkOpenGLCamera (itself a concrete
/// implementation of vtkCamera) that provides improved (or at least clearer) 
/// stereo handling in virtual and mixed reality environments
///
/// \details Camera overrides a number of base class methods to
/// simplify and streamline the calculation of the necessary view and 
/// (off-axis) projection matrices required for proper stereoscopic 
/// visualization.  
/// Details on the theory and implementation can be found in the paper:
/// R.Kooima, Generalized perspective projection, http://csc.lsu.edu/
/// ~kooima/articles/genperspective/index.html, 2008, [Accessed 01 May,
/// 2020].
class Camera : public vtkOpenGLCamera
{
public:
	static Camera* New();
	vtkTypeMacro(Camera, vtkOpenGLCamera);
	void PrintSelf(std::ostream& os, vtkIndent indent) override;

	/// \brief Sets / gets whether view and projection matrix calculations use
	/// eye position values that were explicitly set by the user
	/// \details If set to 'true', view and projection matrix calculations will 
	/// use left and right eye position values set by the user via the 
	/// SetLeftEyePosition / SetRightEyePosition member functions.  If set to
	/// 'false', left and right eye positions will be generated using the
	/// current eye transform matrix
	void SetUseExplicitEyePositions(bool);
	bool GetUseExplicitEyePositions() const;

	/// \brief Methods for setting / getting the right eye position
	/// \note Values set using these methods will only be used when
	/// SetUseExplicitEyePositions is 'true'
	void SetLeftEyePosition(const double[3]);
	void SetLeftEyePosition(double, double, double);
	double* GetLeftEyePosition();
	void GetLeftEyePosition(double[3]) const;
	void GetLeftEyePosition(double&, double&, double&) const;

	/// \brief Methods for setting / getting the left eye position
	/// \note Values set using these methods will only be used when
	/// SetUseExplicitEyePositions is 'true'
	void SetRightEyePosition(const double[3]);
	void SetRightEyePosition(double, double, double);
	double* GetRightEyePosition();
	void GetRightEyePosition(double[3]) const;
	void GetRightEyePosition(double&, double&, double&) const;

	/// \brief Set the world space position of the bottom left corner
	/// of the screen
	void SetScreenBottomLeft(double, double, double) override;

	/// \brief Set the world space position of the bottom right corner
	/// of the screen
	void SetScreenBottomRight(double, double, double) override;
	
	/// \brief Set the world space position of the top right corner
	/// of the screen
	void SetScreenTopRight(double, double, double) override;

	/// \brief Returns the camera's model-view matrix / transform object
	vtkMatrix4x4* GetModelViewTransformMatrix() override;
	vtkTransform* GetModelViewTransformObject() override;

	/// \brief Resets the camera clipping range based on the input 
	/// bounds
	void ResetClippingRange(const double bounds[6]);
	void ResetClippingRange(double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax);

protected:
	/// \brief protected default constructor and destructor to force
	/// heap allocation
	explicit Camera();
	virtual ~Camera() override;

	/// \brief Unfortunately, the VTK base class implementation of 
	/// "ComputeWorldToScreenMatrix" is not virtual, so we define this method
	/// instead to correctly calculate the pose of the display in world space
	void ComputeWorldToDisplayMatrix();

	/// \brief Computes the camera view transform
	virtual void ComputeViewTransform() override;

	/// \brief Computes the left and right eye positions based on the current
	/// eye transformation matrix
	void ComputeEyePositions();

	/// \brief Overrides the calculation of the projection matrices
	virtual void ComputeProjectionTransform(double aspect,
		double nearPlaneZ, double farPlaneZ) override;

	/// \brief Eye positions
	double LeftEyePosition[3];
	double RightEyePosition[3];
	vtkTimeStamp EyePositionsMTime;

	bool UseExplicitEyePositions;

private:
	/// \brief Disallow copy construction and assignment
	Camera(const Camera&) = delete;
	void operator=(const Camera&) = delete;
};

#endif

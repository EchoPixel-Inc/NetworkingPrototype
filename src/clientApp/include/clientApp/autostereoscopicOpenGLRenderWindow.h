#ifndef autostereoscopicOpenGLRenderWindow_h
#define autostereoscopicOpenGLRenderWindow_h

#include <vtkGenericOpenGLRenderWindow.h>

class vtkRenderer;

class AutostereoscopicOpenGLRenderWindow : public vtkGenericOpenGLRenderWindow
{
public:
	static AutostereoscopicOpenGLRenderWindow* New();

	virtual void AddRenderer(vtkRenderer*) override;
	virtual void StereoRenderComplete() override;

protected:
	using Superclass = vtkGenericOpenGLRenderWindow;

	AutostereoscopicOpenGLRenderWindow();
	AutostereoscopicOpenGLRenderWindow(
		const AutostereoscopicOpenGLRenderWindow&) = delete;
	AutostereoscopicOpenGLRenderWindow& operator=(
		const AutostereoscopicOpenGLRenderWindow&) = delete;
};

#endif
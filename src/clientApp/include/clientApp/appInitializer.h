#ifndef appInitializer_h
#define appInitializer_h

#include <memory>

class QWidget;
class QVTKOpenGLWindow;

class AppInitializer
{
public:
	explicit AppInitializer();
	~AppInitializer();

	bool initialize();

private:
	std::unique_ptr<QWidget> m_UserInterface;
	std::unique_ptr<QVTKOpenGLWindow> m_StereoWindow;
};

#endif
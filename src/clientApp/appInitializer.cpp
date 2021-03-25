#include "clientApp/appInitializer.h"
#include "clientApp/clientApp.h"
#include "clientApp/mainWindow.h"
#include "clientApp/applicationActionsWidget.h"

#include "display/displayInterface.h"
#include "display/displayUtilities.h"

#include <QHostAddress>
#include <QGuiApplication>
#include <QVTKOpenGLWindow.h>
//=============================================================================
AppInitializer::AppInitializer() = default;
//=============================================================================

//=============================================================================
AppInitializer::~AppInitializer() = default;
//=============================================================================

//=============================================================================
bool AppInitializer::initialize()
{
	try {
		// ClientApp::instance().launchServerApp();
		auto defaultDisplay = DisplayInterface::getDefaultImplementation();
		std::cout << "Detected display info:\n"
			<< defaultDisplay->getInfo() << std::endl;

		if (defaultDisplay->getInfo().manufacturer == "Barco") {
			// Need an interface compatible with an autostereoscopic display

			m_UserInterface = std::make_unique<ApplicationActionsWidget>();
			m_StereoWindow = std::make_unique<QVTKOpenGLWindow>();
			m_StereoWindow->setRenderWindow(
				ClientApp::instance().getRenderWindow());
			m_StereoWindow->setFormat(QVTKOpenGLWindow::defaultFormat(true));
			m_StereoWindow->setWidth(defaultDisplay->getInfo().size[0]);
			m_StereoWindow->setHeight(defaultDisplay->getInfo().size[1]);
			m_StereoWindow->setFlags(Qt::FramelessWindowHint);
			m_StereoWindow->setPosition(defaultDisplay->getInfo().position[0],
				defaultDisplay->getInfo().position[1]);

			QObject::connect(
				static_cast<QGuiApplication*>(QGuiApplication::instance()),
				&QGuiApplication::focusWindowChanged, m_UserInterface.get(),
				[this](const QWindow* window) {
					if (window) {
						if (window == m_UserInterface->windowHandle()) {
							m_StereoWindow->requestActivate();
						}
					}
				});

			m_StereoWindow->show();
		}
		else {
			auto mainWindow = std::make_unique<MainWindow>();
			mainWindow->setRenderWindow(
				ClientApp::instance().getRenderWindow());
			m_UserInterface.reset(mainWindow.release());
		}

		m_UserInterface->show();

		ClientApp::instance().initGraphics();
		ClientApp::instance().initTracking();
	}
	catch (std::exception& e) {
		std::cout << "Client application failed to initialize" << std::endl;
		return false;
	}

	return true;
}
//=============================================================================

//=============================================================================

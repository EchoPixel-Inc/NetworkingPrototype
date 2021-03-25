#include "clientApp/mainWindow.h"
#include "clientApp/ui_mainWindow.h"
#include "clientApp/clientApp.h"

#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>

#include <QWidget>
#include <QEvent>
#include <QResizeEvent>

#include <limits>
#include <iostream>

//=============================================================================
MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) :
	QMainWindow{parent, flags},
	m_UIActions{this},
	m_Gui{new Ui::MainWindow}
{
	m_Gui->setupUi(this);
	m_Gui->openGLWidget->setFormat(QVTKOpenGLWindow::defaultFormat(true));

	// When the main window receives focus, delegate that focus to the
	// QOpenGLWindow representing the rendering context so that keyboard events
	// are correctly propagated to the interactor, etc.
	QObject::connect(static_cast<QGuiApplication*>(QGuiApplication::instance()),
		&QGuiApplication::focusWindowChanged, this,
		[this](const QWindow* window) {
			if (window) {
				if (window == this->windowHandle()) {
					m_Gui->openGLWidget->embeddedOpenGLWindow()
						->requestActivate();
				}
			}
		});

	// add the actions to the menu
	m_Gui->menuFile->addAction(m_UIActions.openDICOMAction);
	m_Gui->menuNetworking->addAction(m_UIActions.startNetworkSessionAction);
	m_Gui->menuNetworking->addAction(m_UIActions.showPeerConnectionsAction);
	m_Gui->menuActions->addAction(m_UIActions.createWidgetAction);
	m_Gui->menuActions->addAction(m_UIActions.resetVolumeAction);
	m_Gui->menuActions->addAction(m_UIActions.calibrateInteractionDeviceAction);
}
//=============================================================================

//=============================================================================
MainWindow::~MainWindow()
{
	delete m_Gui;
}
//=============================================================================

//=============================================================================
void MainWindow::setRenderWindow(vtkRenderWindow* renWin)
{
	m_Gui->openGLWidget->setRenderWindow(renWin);
}
//=============================================================================

//=============================================================================
void MainWindow::showEvent(QShowEvent* event)
{
	QMainWindow::showEvent(event);

	while (!m_Gui->openGLWidget->isValid()) {
		QApplication::processEvents();
		std::cout << "Processing events until OpenGL context is valid" << std::endl;
	}

	std::cout << "show Event" << std::endl;
	updateRenderWindow();
}
//=============================================================================

//=============================================================================
void MainWindow::moveEvent(QMoveEvent* event)
{
	QMainWindow::moveEvent(event);
	updateRenderWindow();
}
//=============================================================================

//=============================================================================
void MainWindow::updateRenderWindow()
{
	auto topLeftCorner = m_Gui->openGLWidget->mapToGlobal({0, 0});
	auto renderWindow = m_Gui->openGLWidget->renderWindow();
	renderWindow->SetPosition(topLeftCorner.x(), topLeftCorner.y());
	ClientApp::instance().updateScreenPose();
}
//=============================================================================

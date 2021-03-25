#ifndef mainWindow_h
#define mainWindow_h

#include "clientApp/uiActions.h"

#include <QMainWindow>

#include <memory>

class PeerConnectionWindow;
class NetworkSessionSelectionDialog;

namespace Ui
{
class MainWindow;
}

class vtkRenderWindow;

class QWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT;

public:
	explicit MainWindow(
		QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

	virtual ~MainWindow();

	void setRenderWindow(vtkRenderWindow* renWin);

protected:
	void showEvent(QShowEvent* showEvent) override;
	void moveEvent(QMoveEvent* moveEvent) override;
	void updateRenderWindow();

private:
	Ui::MainWindow* m_Gui;
	UIActions m_UIActions;
};

#endif
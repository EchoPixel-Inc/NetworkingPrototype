#include "clientApp/ApplicationActionsWidget.h"
#include "clientApp/ui_applicationActionsWidget.h"

#include <QApplication>

//==============================================================================
ApplicationActionsWidget::ApplicationActionsWidget(
	QWidget* parent, Qt::WindowFlags flags) :
	QWidget{parent},
	m_UIActions{this},
	m_Gui{std::make_unique<Ui::ApplicationActionsWidget>()}
{
	m_Gui->setupUi(this);

	m_Gui->loadDICOMToolButton->setDefaultAction(m_UIActions.openDICOMAction);
	m_Gui->startNetworkSessionToolButton->setDefaultAction(
		m_UIActions.startNetworkSessionAction);
	m_Gui->showPeersToolButton->setDefaultAction(
		m_UIActions.showPeerConnectionsAction);
	m_Gui->createWidgetToolButton->setDefaultAction(
		m_UIActions.createWidgetAction);
	m_Gui->resetVolumeToolButton->setDefaultAction(
		m_UIActions.resetVolumeAction);
	m_Gui->calibrateInteractionDeviceToolButton->setDefaultAction(
		m_UIActions.calibrateInteractionDeviceAction);
}
//==============================================================================

//==============================================================================
ApplicationActionsWidget::~ApplicationActionsWidget() = default;
//==============================================================================

//==============================================================================
void ApplicationActionsWidget::closeEvent(QCloseEvent* event)
{
	QApplication::quit();
}
//==============================================================================
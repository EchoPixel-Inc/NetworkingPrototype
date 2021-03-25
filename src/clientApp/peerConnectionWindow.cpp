#include "clientApp/peerConnectionWindow.h"
#include "clientApp/ui_peerConnectionWindow.h"
#include "clientApp/peerDelegate.h"
#include "clientApp/clientApp.h"

#include <QHostAddress>
#include <QStandardItemModel>
#include <QValidator>

//=============================================================================
PeerConnectionWindow::PeerConnectionWindow(QWidget* parent) :
	QWidget{ parent },
	m_Gui{std::make_unique<Ui::PeerConnectionWindow>()}
{
	m_Gui->setupUi(this);
	setWindowTitle("Peer Connections");

	m_Gui->peerListView->setItemDelegate(new PeerDelegate);

	QObject::connect(m_Gui->disconnectPushbutton, &QPushButton::clicked,
		this, &PeerConnectionWindow::onDisconnectButtonPushed);
}
//=============================================================================

//=============================================================================
PeerConnectionWindow::~PeerConnectionWindow() = default;
//=============================================================================

//=============================================================================
void PeerConnectionWindow::setPeerModel(QStandardItemModel* peerModel)
{
	m_Gui->peerListView->setModel(peerModel);
}
//=============================================================================

//=============================================================================
void PeerConnectionWindow::onDisconnectButtonPushed()
{
	ClientApp::instance().closeSession();
}
//=============================================================================


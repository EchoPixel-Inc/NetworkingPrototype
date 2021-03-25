#include "clientApp/networkSessionSelectionDialog.h"
#include "clientApp/ui_networkSessionSelectionDialog.h"

//=============================================================================
NetworkSessionSelectionDialog::NetworkSessionSelectionDialog(QWidget* parent) :
	QDialog{parent},
	m_Gui{std::make_unique<Ui::NetworkSessionSelectionDialog>()},
	m_SessionType{SessionType::Join}
{
	m_Gui->setupUi(this);
	setWindowModality(Qt::WindowModality::ApplicationModal);
	setWindowTitle("Session Selection");

	QObject::connect(
		m_Gui->startSessionPushButton, &QPushButton::clicked, this, [this] {
			m_SessionType = SessionType::Start;
			accept();
		});

	QObject::connect(
		m_Gui->joinSessionPushButton, &QPushButton::clicked, this, [this] {
			m_SessionType = SessionType::Join;
			accept();
		});
}
//=============================================================================

//=============================================================================
NetworkSessionSelectionDialog::~NetworkSessionSelectionDialog() = default;
//=============================================================================

//=============================================================================
auto NetworkSessionSelectionDialog::getSessionSelection() const -> SessionType
{
	return m_SessionType;
}
//=============================================================================

//=============================================================================
void NetworkSessionSelectionDialog::setEnableStartSession(bool enable)
{
	m_Gui->startSessionPushButton->setEnabled(enable);
}
//=============================================================================

//=============================================================================
void NetworkSessionSelectionDialog::setEnableJoinSession(bool enable)
{
	m_Gui->joinSessionPushButton->setEnabled(enable);
}
//=============================================================================
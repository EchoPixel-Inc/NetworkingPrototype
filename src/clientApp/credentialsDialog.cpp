#include "clientApp/credentialsDialog.h"
#include "clientApp/ui_credentialsDialog.h"

#include <QString>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>

//=============================================================================
CredentialsDialog::CredentialsDialog(QWidget* parent) : 
	QDialog{ parent },
	m_Gui{ new Ui::CredentialsDialog }
{
	m_Gui->setupUi(this);
	m_Gui->sessionKeyLineEdit->setEchoMode(QLineEdit::Normal);

	setWindowTitle("Enter Credentials");

	QObject::connect(m_Gui->hideSessionKeyCheckbox, &QCheckBox::stateChanged,
		[this](int state) {
			switch (state) {
				case Qt::Unchecked: {
					m_Gui->sessionKeyLineEdit->setEchoMode(QLineEdit::Normal);
					break;
				}
				case Qt::Checked: {
					m_Gui->sessionKeyLineEdit->setEchoMode(QLineEdit::Password);
					break;
				}
			}
		});
}
//=============================================================================

//=============================================================================
void CredentialsDialog::setName(const QString& name)
{
	m_Gui->nameLineEdit->setText(name);
}
//=============================================================================

//=============================================================================
QString CredentialsDialog::getSessionKey() const
{
	return m_Gui->sessionKeyLineEdit->text();
}
//=============================================================================

//=============================================================================
QString CredentialsDialog::getName() const
{
	return m_Gui->nameLineEdit->text();
}
//=============================================================================

//=============================================================================
void CredentialsDialog::accept()
{
	if (getName().isEmpty()) {
		QMessageBox emptyNameWarningMsgBox;
		emptyNameWarningMsgBox.setText("Please choose a name");
		emptyNameWarningMsgBox.exec();
	}
	else {
		QDialog::accept();
	}
}
//=============================================================================

//=============================================================================
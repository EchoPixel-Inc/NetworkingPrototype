#include "clientApp/networkSessionConnectionDialog.h"
#include "clientApp/ui_networkSessionConnectionDialog.h"

#include <QValidator>

namespace {
class PortValidator : public QValidator
{
public:
	explicit PortValidator(QObject* parent = nullptr) : QValidator{ parent } {}
	QValidator::State validate(QString& input, int& position) const override
	{
		QRegExp re("^[0-9][0-9]\?[0-9]\?[0-9]\?[0-9]\?");
		if (re.exactMatch(input)) {
			auto portNum = input.toInt();
			if ((portNum < 1) || (portNum > 65535)) {
				return QValidator::Invalid;
			}
			else {
				return QValidator::Acceptable;
			}
		}
		else {
			return QValidator::Invalid;
		}
	}

	void fixup(QString& input) const override {}
};
} // end anonymous namespace

//==============================================================================
NetworkSessionConnectionDialog::NetworkSessionConnectionDialog(QWidget* parent) : 
	QDialog{ parent },
	m_Gui{ std::make_unique<Ui::NetworkSessionConnectionDialog>() }
{
	m_Gui->setupUi(this);

	setWindowModality(Qt::WindowModality::ApplicationModal);
	setWindowTitle("Connection Info");

	m_Gui->hostAddressIPv4Widget->setIPv4Address(QHostAddress::LocalHost);
	m_Gui->portNumberLineEdit->setText(QString::number(4890));
	m_Gui->portNumberLineEdit->setValidator(new PortValidator(
		m_Gui->portNumberLineEdit));

	QObject::connect(m_Gui->startSessionPushButton, &QPushButton::clicked,
		this, &NetworkSessionConnectionDialog::accept);

	QObject::connect(m_Gui->cancelPushButton, &QPushButton::clicked,
		this, &NetworkSessionConnectionDialog::reject);
}
//==============================================================================

//==============================================================================
NetworkSessionConnectionDialog::~NetworkSessionConnectionDialog() = default;
//==============================================================================

//==============================================================================
QHostAddress NetworkSessionConnectionDialog::getHostAddress() const
{
	return m_Gui->hostAddressIPv4Widget->getIPv4Address();
}
//==============================================================================

//==============================================================================
quint16 NetworkSessionConnectionDialog::getPortNumber() const
{
	return static_cast<quint16>(m_Gui->portNumberLineEdit->text().toInt());
}
//==============================================================================
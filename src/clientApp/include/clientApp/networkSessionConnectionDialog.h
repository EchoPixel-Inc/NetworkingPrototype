#ifndef networkSessionConnectionDialog_h
#define networkSessionConnectionDialog_h

#include <QDialog>
#include <QHostAddress>

#include <memory>

namespace Ui {
	class NetworkSessionConnectionDialog;
}

class NetworkSessionConnectionDialog : public QDialog
{
public:
	enum class SessionType { Start, Join };

	NetworkSessionConnectionDialog(QWidget* parent = nullptr);
	virtual ~NetworkSessionConnectionDialog();

	QHostAddress getHostAddress() const;
	quint16 getPortNumber() const;

private:
	std::unique_ptr<Ui::NetworkSessionConnectionDialog> m_Gui;
};

#endif
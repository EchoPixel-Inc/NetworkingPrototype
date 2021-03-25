#ifndef networkSessionSelectionDialog_h
#define networkSessionSelectionDialog_h

#include <QDialog>

#include <memory>

namespace Ui {
	class NetworkSessionSelectionDialog;
}

class NetworkSessionSelectionDialog : public QDialog
{
public:
	enum class SessionType { Start, Join };

	NetworkSessionSelectionDialog(QWidget* parent = nullptr);
	virtual ~NetworkSessionSelectionDialog();

	void setEnableStartSession(bool);
	void setEnableJoinSession(bool);

	SessionType getSessionSelection() const;

private:
	SessionType m_SessionType;
	std::unique_ptr<Ui::NetworkSessionSelectionDialog> m_Gui;
};

#endif
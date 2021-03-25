#ifndef credentialsDialog_h
#define credentialsDialog_h

#include <QDialog>

namespace Ui
{
class CredentialsDialog;
}

class CredentialsDialog : public QDialog
{
public:
	explicit CredentialsDialog(QWidget* parent = nullptr);
	virtual ~CredentialsDialog() = default;

	void setName(const QString&);

	QString getSessionKey() const;
	QString getName() const;

	virtual void accept() override;

private:
	Ui::CredentialsDialog* m_Gui;
};

#endif
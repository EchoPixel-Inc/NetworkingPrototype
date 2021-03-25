#ifndef peerConnectionWindow_h
#define peerConnectionWindow_h

#include <QWidget>

class QHostAddress;
class QStandardItemModel;

#include <memory>

namespace Ui
{
class PeerConnectionWindow;
}

class PeerConnectionWindow : public QWidget
{
	Q_OBJECT;

public:
	explicit PeerConnectionWindow(QWidget* parent = nullptr);
	virtual ~PeerConnectionWindow();

	void setPeerModel(QStandardItemModel* peerModel);

protected slots:
	void onDisconnectButtonPushed();

private:
	std::unique_ptr<Ui::PeerConnectionWindow> m_Gui;
};

#endif
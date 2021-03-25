#ifndef uiActions_h
#define uiActions_h

class PeerConnectionWindow;
class NetworkSessionSelectionDialog;

#include <QAction>
#include <QObject>
#include <QWidget>

class UIActions : public QObject {
	Q_OBJECT;

public:
	explicit UIActions(QWidget* parent = nullptr);
	~UIActions();

	QAction* openDICOMAction;
	QAction* resetVolumeAction;
	QAction* createWidgetAction;
	QAction* startNetworkSessionAction;
	QAction* showPeerConnectionsAction;
	QAction* calibrateInteractionDeviceAction;

	PeerConnectionWindow* peerConnectionsWindow;
	std::unique_ptr<NetworkSessionSelectionDialog> sessionSelectionDialog;
	QMetaObject::Connection onServerStartedConnection;
	QWidget* parent;

protected slots:
	void onNetworkSessionRequested();
	void onLoadDICOM();
};

#endif
#ifndef clientApp_h
#define clientApp_h

#include "networking/networkMessage.h"
#include "common/coreTypes.h"
#include "appcore/applicationObjects.h"
#include "appcore/messageEncoder.h"
#include "clientApp/trackingManager.h"

#include <vtkSmartPointer.h>

#include <QObject>
#include <QHostAddress>
#include <QStandardItemModel>
#include <QTimer>
#include <QProcess>

#include <memory>
#include <string>
#include <optional>

class Connection;
class Interactor;
class vtkRenderWindow;
class vtkActor;
class vtkOrientationMarkerWidget;
class vtkImageData;

class ClientApp : public QObject
{
	Q_OBJECT;

public:
	using IdType = common::IdType;

	static ClientApp& instance();
	~ClientApp();

	ClientApp(const ClientApp&) = delete;
	ClientApp& operator=(const ClientApp&) = delete;

	void connectToHost(const QHostAddress& hostAddress, quint16 portNumber);
	void closeSession();

	void sendCredentials(const std::string& sessionCode,
		const std::string& nickname);

	void launchServerApp(const QHostAddress& hostAddress =
		QHostAddress::LocalHost, quint16 portNumber = 3760);

	vtkRenderWindow* getRenderWindow() const;
	QStandardItemModel& getPeerConnectionModel();

	void setImageData(vtkSmartPointer<vtkImageData>);

	void calibrateInteractionDevice();
	void initGraphics();
	void initTracking();

	// \brief A convenience method for resetting the volume position
	// (may want to consider exposing the volume widget directly or otherwise
	// wrapping it in some type of manager object)
	void resetVolume();
	void createWidget();
	void updateScreenPose();

signals:
	void connectionStarted(QPrivateSignal);
	void connectionError(const QString&, QPrivateSignal);
	void connectionEnded(QPrivateSignal);
	void credentialsRequested(QPrivateSignal);
	void serverError(const QString&, QPrivateSignal);
	void serverStarted(QPrivateSignal);
	void serverFinished(QPrivateSignal);
	void serverStatusChanged(QProcess::ProcessState, QPrivateSignal);
	void widgetPlacementEnded(QPrivateSignal);

protected:
	using MessageType = NetworkMessage;
	using ColorVectorType = common::ColorVectorType;

	void sendMessage(const NetworkMessage&);

	void onCredentialsRequested();
	void onAuthorizationSucceeded(const PeerInfo&);
	void onAuthorizationFailed();
	void onPeerAdded(const PeerInfo&);
	void onPeerRemoved(const PeerInfo&);
	void onDisconnected();
	void onLaserUpdated(const LaserUpdate&);
	void onVolumeUpdated(const VolumeUpdate&);
	void onWidgetUpdated(const WidgetUpdate&);
	void onPlaneUpdated(const PlaneUpdate&);
	void onFullStateUpdated(const std::vector<PeerInfo>&,
		ApplicationObjects&&);

private:
	explicit ClientApp();

	ApplicationObjects m_ApplicationObjects;
	MessageEncoder m_MessageEncoder;
	TrackingManager m_TrackingManager;
	QStandardItemModel m_ConnectedPeerModel;
	QTimer m_RenderTimer;
	vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
	vtkSmartPointer<vtkOrientationMarkerWidget> m_OrientationMarker;
	vtkSmartPointer<Interactor> m_Interactor;
	std::unique_ptr<Connection> m_Connection;
	std::unique_ptr<QProcess> m_ServerProcess;
	std::optional<unsigned long> m_ClientId;
};

#endif
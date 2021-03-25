#ifndef leapMotionClient_h
#define leapMotionClient_h

#include "tracking/trackingTypes.h"

#include <QObject>
#include <QList>
#include <QSslError>
#include <QUrl>
#include <QJsonObject>
#include <QWebSocket>

#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>
#include <functional>

namespace tracking
{
class LeapMotionClient : public QObject
{
	Q_OBJECT;

public:
	explicit LeapMotionClient(const QUrl&, QObject* parent = nullptr);
	~LeapMotionClient();

	LeapMotionClient(const LeapMotionClient&) = delete;
	LeapMotionClient& operator=(const LeapMotionClient&) = delete;

	LeapMotionClient(LeapMotionClient&&) = default;
	LeapMotionClient& operator=(LeapMotionClient&&) = default;

	DevicePoseType getCurrentHandPose() const;

	void setDeviceMoveCallback(std::function<void(const DevicePoseType&)>);
	void setDeviceButtonPressCallback(std::function<void()>);
	void setDeviceButtonReleaseCallback(std::function<void()>);

public slots:
	void connectToHost();

protected slots:
	void onConnected();
	void onTextMessageReceived(const QString& message);
	void onTextFrameReceived(const QString& frame, bool isLastFrame);
	void onError(QAbstractSocket::SocketError error);
	void onStateChanged(QAbstractSocket::SocketState state);

signals:
	void error(const QString&, QPrivateSignal);

protected:
	void processFrameData(const QJsonObject&);
	void processDeviceEvent(const QJsonObject&);

private:
	class PinchDetector;
	std::unique_ptr<PinchDetector> m_PinchDetector;

	mutable std::mutex m_CallbackMutex;
	mutable std::shared_mutex m_PoseMutex;
	DevicePoseType m_CurrentHandPose;
	std::function<void(const DevicePoseType&)> m_DeviceMoveCallback;
	std::function<void()> m_DeviceButtonPressCallback;
	std::function<void()> m_DeviceButtonReleaseCallback;
	std::unique_ptr<QWebSocket> m_WebSocket;
	QUrl m_HostUrl;
};
}  // namespace tracking

#endif

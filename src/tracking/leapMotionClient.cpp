#include "tracking/leapMotionClient.h"
#include "common/coreTypes.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <string>
#include <iostream>
#include <optional>
#include <array>

namespace tracking
{
class LeapMotionClient::PinchDetector
{
public:
	explicit PinchDetector() = default;
	~PinchDetector() = default;

	void update(const QJsonArray& pointableArray)
	{
		if (pointableArray.size() == 0) {
			return;
		}

		using PositionType = common::Point3dType;
		std::optional<PositionType> thumbPosition;
		std::optional<PositionType> indexPosition;

		for (unsigned int i = 0; i < pointableArray.size(); i++) {
			auto pointableObj = pointableArray[i].toObject();

			if (pointableObj.isEmpty()) {
				continue;
			}

			auto tipArray = pointableObj.value("tipPosition").toArray();
			PositionType tipPositionArray{tipArray[0].toDouble(),
				tipArray[1].toDouble(), tipArray[2].toDouble()};

			auto pointableType = pointableObj.value("type");
			if (pointableType == 0) {  // thumb
				thumbPosition = tipPositionArray;
			}
			else if (pointableType == 1) {	// index finger
				indexPosition = tipPositionArray;
			}
		}

		// If we found both pointables, check the distance between the two
		if (thumbPosition.has_value() && indexPosition.has_value()) {
			auto distance =
				(thumbPosition.value() - indexPosition.value()).norm();
			if (distance < 30.0) {
				m_IsPinching = true;
			}
			else {
				m_IsPinching = false;
			}
		}
	}

	bool isPinching() const { return m_IsPinching; };

private:
	bool m_IsPinching = false;
};

//=============================================================================
LeapMotionClient::LeapMotionClient(const QUrl& url, QObject* parent) :
	QObject{parent},
	m_HostUrl{url},
	m_WebSocket{nullptr},
	m_CurrentHandPose{DevicePoseType::Identity()},
	m_PinchDetector{std::make_unique<PinchDetector>()}
{}
//=============================================================================

//=============================================================================
LeapMotionClient::~LeapMotionClient() = default;
//=============================================================================

//=============================================================================
auto LeapMotionClient::getCurrentHandPose() const -> DevicePoseType
{
	std::shared_lock<std::shared_mutex> locker(m_PoseMutex);
	return m_CurrentHandPose;
}
//=============================================================================

//=============================================================================
void LeapMotionClient::setDeviceMoveCallback(
	std::function<void(const DevicePoseType&)> clbk)
{
	std::lock_guard<std::mutex> lock(m_CallbackMutex);
	m_DeviceMoveCallback = clbk;
}
//=============================================================================

//=============================================================================
void LeapMotionClient::setDeviceButtonPressCallback(std::function<void()> clbk)
{
	std::lock_guard<std::mutex> lock(m_CallbackMutex);
	m_DeviceButtonPressCallback = clbk;
}
//=============================================================================

//=============================================================================
void LeapMotionClient::setDeviceButtonReleaseCallback(
	std::function<void()> clbk)
{
	std::lock_guard<std::mutex> lock(m_CallbackMutex);
	m_DeviceButtonReleaseCallback = clbk;
}
//=============================================================================

//=============================================================================
void LeapMotionClient::connectToHost()
{
	m_WebSocket = std::make_unique<QWebSocket>();

	QObject::connect(m_WebSocket.get(),
		QOverload<QAbstractSocket::SocketState>::of(&QWebSocket::stateChanged),
		this, &LeapMotionClient::onStateChanged);

	QObject::connect(m_WebSocket.get(), &QWebSocket::connected, this,
		&LeapMotionClient::onConnected);

	QObject::connect(m_WebSocket.get(),
		QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
		&LeapMotionClient::onError);

	m_WebSocket->open(m_HostUrl);
}
//=============================================================================

//=============================================================================
void LeapMotionClient::onConnected()
{
	QObject::connect(m_WebSocket.get(), &QWebSocket::textMessageReceived, this,
		&LeapMotionClient::onTextMessageReceived);
}
//=============================================================================

//=============================================================================
void LeapMotionClient::onTextFrameReceived(const QString& message, bool)
{
	onTextMessageReceived(message);
}
//=============================================================================

//=============================================================================
void LeapMotionClient::onTextMessageReceived(const QString& message)
{
	auto jsonDoc = QJsonDocument::fromJson(message.toUtf8());

	if (!jsonDoc.isNull() && jsonDoc.isObject()) {
		auto jsonObj = jsonDoc.object();

		auto messageId = jsonObj.value("id");
		if (messageId != QJsonValue::Undefined) {
			processFrameData(jsonObj);
		}
		else {
			processDeviceEvent(jsonObj);
		}
	}
}
//=============================================================================

//=============================================================================
void LeapMotionClient::processDeviceEvent(const QJsonObject& jsonObj)
{
	bool isAttached{false};
	bool isStreaming{false};
	std::string deviceId{};

	auto parseStateObjFcn = [&isAttached, &isStreaming, &deviceId](
								const QJsonObject& jsonObj) {
		auto attachedElem = jsonObj.value("attached");
		if (attachedElem != QJsonValue::Undefined) {
			isAttached = attachedElem.toBool();
		}
		else {
			return false;
		}

		auto deviceIdElem = jsonObj.value("id");
		if (deviceIdElem != QJsonValue::Undefined) {
			deviceId = deviceIdElem.toString().toStdString();
		}
		else {
			return false;
		}

		auto streamingElem = jsonObj.value("streaming");
		if (streamingElem != QJsonValue::Undefined) {
			isStreaming = streamingElem.toBool();
		}
		else {
			return false;
		}

		return true;
	};

	auto parseEventObjFcn = [&parseStateObjFcn](const QJsonObject& jsonObj) {
		auto stateElem = jsonObj.value("state");
		if ((stateElem == QJsonValue::Undefined) || (!stateElem.isObject())) {
			return false;
		}

		return parseStateObjFcn(stateElem.toObject());
	};

	auto parseBaseObjFcn = [&parseEventObjFcn](const QJsonObject& jsonObj) {
		auto eventElem = jsonObj.value("event");
		if ((eventElem == QJsonValue::Undefined) || (!eventElem.isObject())) {
			return false;
		}

		return parseEventObjFcn(eventElem.toObject());
	};

	if (parseBaseObjFcn(jsonObj)) {
		std::cout << "Client::processDeviceEvent - "
				  << "deviceId: " << deviceId << ", "
				  << "is attached: " << isAttached << ", "
				  << "is streaming: " << isStreaming << std::endl;
	}
}
//=============================================================================

//=============================================================================
void LeapMotionClient::processFrameData(const QJsonObject& jsonObj)
{
	auto handsArray = jsonObj.value("hands").toArray();
	if (handsArray.size() == 0) {
		return;
	}

	int handIndex{0};

	// Check to see if we have a left hand, right hand or both
	if (handsArray.size() > 1) {
		// If we have multiple hands, just use the right hand data
		for (unsigned int i = 0; i < handsArray.size(); i++) {
			auto hand = handsArray[i];
			auto handObj = hand.toObject();

			auto handType = handObj.value("type");
			if (handType != QJsonValue::Undefined) {
				if (handType.toString() == "right") {
					handIndex = i;
				}
			}
		}
	}

	auto arrayElem = handsArray[handIndex];
	if (arrayElem.isObject()) {
		auto handObj = arrayElem.toObject();

		// First verify that this hand object is valid
		auto handValidVal = handObj.value("valid");
		if (handValidVal != QJsonValue::Undefined) {
			if (!handValidVal.toBool()) {
				return;
			}
		}

		DevicePoseType::VectorType palmDirection;
		DevicePoseType::VectorType palmNormal;
		DevicePoseType::VectorType palmRight;
		DevicePoseType::VectorType palmPosition;

		auto palmDirectionVal = handObj.value("direction");
		if (palmDirectionVal != QJsonValue::Undefined) {
			auto palmDirectionVector = palmDirectionVal.toArray();

			palmDirection(0) = palmDirectionVector[0].toDouble();
			palmDirection(1) = palmDirectionVector[1].toDouble();
			palmDirection(2) = palmDirectionVector[2].toDouble();
		}
		else {
			return;
		}

		auto palmNormalVal = handObj.value("palmNormal");
		if (palmNormalVal != QJsonValue::Undefined) {
			auto palmNormalVector = palmNormalVal.toArray();

			palmNormal(0) = palmNormalVector[0].toDouble();
			palmNormal(1) = palmNormalVector[1].toDouble();
			palmNormal(2) = palmNormalVector[2].toDouble();
		}
		else {
			return;
		}

		palmRight = palmNormal.cross(palmDirection);
		palmRight.normalize();

		auto palmPositionVal = handObj.value("palmPosition");
		if (palmPositionVal != QJsonValue::Undefined) {
			auto palmPositionVector = palmPositionVal.toArray();

			palmPosition(0) = palmPositionVector[0].toDouble();
			palmPosition(1) = palmPositionVector[1].toDouble();
			palmPosition(2) = palmPositionVector[2].toDouble();
		}
		else {
			return;
		}

		DevicePoseType devicePose{DevicePoseType::Identity()};

		devicePose.linear().col(0) = palmRight;
		devicePose.linear().col(1) =
			-1.0 * palmNormal;	// want an upward-facing normal
		devicePose.linear().col(2) =
			-1.0 * palmDirection;  // want normal facing into the display
		devicePose.translation() = palmPosition;

		{
			std::lock_guard<std::shared_mutex> poseLock(m_PoseMutex);
			m_CurrentHandPose = devicePose;
		}

		std::lock_guard<std::mutex> lock(m_CallbackMutex);
		if (m_DeviceMoveCallback) {
			m_DeviceMoveCallback(devicePose);
		}
	}

	// Now we check the pinch detector to see if we are starting or
	// ending a pinch gesture
	auto previousPinchStatus = m_PinchDetector->isPinching();
	auto newPinchStatus = previousPinchStatus;

	auto pointablesArray = jsonObj.value("pointables").toArray();
	if (handsArray.size() != 0) {
		m_PinchDetector->update(pointablesArray);
		newPinchStatus = m_PinchDetector->isPinching();
	}

	if (newPinchStatus != previousPinchStatus) {
		std::lock_guard<std::mutex> lock(m_CallbackMutex);
		if (newPinchStatus) {
			if (m_DeviceButtonPressCallback) {
				m_DeviceButtonPressCallback();
			}
		}
		else {
			if (m_DeviceButtonReleaseCallback) {
				m_DeviceButtonReleaseCallback();
			}
		}
	}
}
//=============================================================================

//=============================================================================
void LeapMotionClient::onError(QAbstractSocket::SocketError)
{
	auto errorString = m_WebSocket->errorString();
	emit error(errorString, QPrivateSignal());
}
//=============================================================================

//=============================================================================
void LeapMotionClient::onStateChanged(QAbstractSocket::SocketState state)
{
	std::string newStateString;

	switch (state) {
		case QAbstractSocket::UnconnectedState:
			newStateString.assign("Unconnected State");
			break;
		case QAbstractSocket::HostLookupState:
			newStateString.assign("Host Lookup State");
			break;
		case QAbstractSocket::ConnectingState:
			newStateString.assign("Connecting State");
			break;
		case QAbstractSocket::ConnectedState:
			newStateString.assign("Connected State");
			break;
		case QAbstractSocket::BoundState:
			newStateString.assign("Bound State");
			break;
		case QAbstractSocket::ClosingState:
			newStateString.assign("Closing State");
			break;
		case QAbstractSocket::ListeningState:
			newStateString.assign("Listening State");
			break;
		default: newStateString.assign("Unknown State");
	}

	std::cout << "LeapMotion client - "
			  << " socket state changed to: " << newStateString << std::endl;
}
//=============================================================================
}  // namespace tracking
#include "tracking/leapMotionInteractionDevice.h"
#include "tracking/leapMotionClient.h"
#include "tracking/ewmaFilter.h"

#include <QThread>
#include <QObject>

#include <iostream>

namespace tracking
{
//=============================================================================
LeapMotionInteractionDevice::LeapMotionInteractionDevice(
	const QHostAddress& hostAddress) :
	m_TrackingThread{std::make_unique<QThread>()},
	m_ContextObj{std::make_unique<QObject>()}
{
	int portNumber{6437};
	QString urlString{"ws://"};
	urlString.append(hostAddress.toString());
	urlString.append(":");
	urlString.append(QString::number(portNumber));
	urlString.append("/v6.json");

	QUrl leapServiceUrl{urlString, QUrl::ParsingMode::StrictMode};
	if (!leapServiceUrl.isValid()) {
		throw std::runtime_error{"Leap Service host address is invalid"};
	}

	m_LeapClient = std::make_unique<LeapMotionClient>(leapServiceUrl);

	QObject::connect(m_LeapClient.get(), &LeapMotionClient::error,
		m_ContextObj.get(), [this](const QString& errorString) {
			std::cerr << "LeapMotion interaction device error: "
					  << errorString.toStdString() << std::endl;
		});

	QObject::connect(m_TrackingThread.get(), &QThread::started,
		m_LeapClient.get(), &LeapMotionClient::connectToHost);

	m_LeapClient->moveToThread(m_TrackingThread.get());
	m_TrackingThread->start();
}
//=============================================================================

//=============================================================================
LeapMotionInteractionDevice::~LeapMotionInteractionDevice()
{
	if (m_TrackingThread) {
		m_TrackingThread->quit();

		if (!m_TrackingThread->wait(3000)) {
			std::cerr << "LeapMotion interaction device tracking thread "
					  << "did not shut down properly" << std::endl;

			m_TrackingThread->terminate();
			m_TrackingThread->wait();
		}
	}
}
//=============================================================================

//=============================================================================
auto LeapMotionInteractionDevice::getPose() const -> DevicePoseType
{
	return m_LeapClient->getCurrentHandPose();
}
//=============================================================================

//=============================================================================
void LeapMotionInteractionDevice::setDeviceMovedCallback(MoveCallbackType clbk)
{
	auto ewmaFilter = EWMAFilter();
	ewmaFilter.setAlpha(0.15);

	auto callbackWrapper = [clbk, filter = std::move(ewmaFilter)](
							   const DevicePoseType& devicePose) mutable {
		const auto& filteredDevicePose = filter.update(devicePose);
		std::invoke(clbk, filteredDevicePose);
	};

	m_LeapClient->setDeviceMoveCallback(callbackWrapper);
}
//=============================================================================

//=============================================================================
void LeapMotionInteractionDevice::setButtonPressCallback(
	ButtonPressCallbackType clbk)
{
	m_LeapClient->setDeviceButtonPressCallback(clbk);
}
//=============================================================================

//=============================================================================
void LeapMotionInteractionDevice::setButtonReleaseCallback(
	ButtonReleaseCallbackType clbk)
{
	m_LeapClient->setDeviceButtonReleaseCallback(clbk);
}
//=============================================================================
}  // namespace tracking
#include "clientApp/trackingManager.h"
#include "clientApp/trackerEventProcessor.h"
#include "tracking/interactionDeviceInterface.h"
#include "tracking/headTargetInterface.h"
#include "tracking/zSpaceInteractionDeviceBuilder.h"
#include "tracking/zSpaceHeadTargetBuilder.h"
#include "tracking/leapMotionInteractionDeviceBuilder.h"
#include "tracking/vrInkInteractionDeviceBuilder.h"
#include "tracking/barcoHeadTargetBuilder.h"
#include "interaction/customQEvents.h"

#include <QCoreApplication>
#include <QHostAddress>

#include <sstream>
#include <iostream>

//=============================================================================
TrackingManager::TrackingManager() :
	m_InteractionDevice{nullptr},
	m_EventProcessor{std::make_unique<TrackerEventProcessor>()}
{}
//=============================================================================

//=============================================================================
TrackingManager::~TrackingManager() = default;
//=============================================================================

//=============================================================================
void TrackingManager::setInteractor(Interactor* interactor)
{
	m_EventProcessor->setInteractor(interactor);
}
//=============================================================================

//=============================================================================
void TrackingManager::initializeHeadTracking(const std::string& headTargetType)
{
	if (headTargetType == "zspace") {
		m_HeadTarget = tracking::zSpaceHeadTargetBuilder().create();
	}
	else if (headTargetType == "barco") {
		m_HeadTarget = tracking::BarcoHeadTargetBuilder().create();
	}
	else {
		std::stringstream errorStream;
		errorStream << "Error initializing head target: "
					<< "'" << headTargetType << "'"
					<< " is not a supported head target";

		throw std::runtime_error(errorStream.str());
	}

	std::cout << "Successfully initialized " << headTargetType << " head target"
		<< std::endl;
}
//=============================================================================

//=============================================================================
void TrackingManager::initializeInteractionDevice(
	const std::string& interactionDeviceType)
{
	if (interactionDeviceType == "zspace") {
		m_InteractionDevice =
			tracking::zSpaceInteractionDeviceBuilder().create();
	}
	else if (interactionDeviceType == "leap_motion") {
		m_InteractionDevice = tracking::LeapMotionInteractionDeviceBuilder(
			QHostAddress::LocalHost)
								  .create();
	}
	else if (interactionDeviceType == "logitech_vr_ink") {
		m_InteractionDevice =
			tracking::VRInkInteractionDeviceBuilder().create();
	}
	else {
		std::stringstream errorStream;
		errorStream << "Error initializing interaction device: "
					<< "'" << interactionDeviceType << "'"
					<< " is not a supported device";

		throw std::runtime_error(errorStream.str());
	}

	m_InteractionDeviceResources =
		std::make_shared<InteractionDeviceResources>();

	m_InteractionDevice->setDeviceMovedCallback(
		[this, resources = m_InteractionDeviceResources](
			const tracking::DevicePoseType& devicePose) {
			auto pose = devicePose;

			{
				std::lock_guard<std::mutex> lock(resources->mutex);
				if (resources->calibrationTransform.has_value()) {
					pose = resources->calibrationTransform.value() * devicePose;
				}
			}

			auto moveEvent = new DeviceMoveEvent(pose);
			QCoreApplication::removePostedEvents(
				m_EventProcessor.get(), moveEvent->type());

			QCoreApplication::postEvent(m_EventProcessor.get(), moveEvent);
		});

	m_InteractionDevice->setButtonPressCallback([this] {
		auto buttonPressEvent = new DeviceButtonPressEvent();
		QCoreApplication::postEvent(m_EventProcessor.get(), buttonPressEvent);
	});

	m_InteractionDevice->setButtonReleaseCallback([this] {
		auto buttonReleaseEvent = new DeviceButtonReleaseEvent();
		QCoreApplication::postEvent(m_EventProcessor.get(), buttonReleaseEvent);
	});

	std::cout << "Successfully initialized " << interactionDeviceType 
		<< " interaction device" << std::endl;
}
//=============================================================================

//=============================================================================
void TrackingManager::calibrateInteractionDevice()
{
	if (m_InteractionDevice) {
		auto currentPose = m_InteractionDevice->getPose();

		std::lock_guard<std::mutex> lock(m_InteractionDeviceResources->mutex);
		m_InteractionDeviceResources->calibrationTransform =
			currentPose.inverse();
	}
}
//=============================================================================

//=============================================================================
auto TrackingManager::getCurrentHeadPose() const -> HeadPoseType
{
	if (m_HeadTarget) {
		return m_HeadTarget->getHeadPosition();
	}

	HeadPoseType defaultPose = HeadPoseType::Identity();
	defaultPose.translation() = HeadPoseType::VectorType{70.0, 80.0, 400.0};

	return defaultPose;
}
//=============================================================================

//=============================================================================
